#include "UAVManager.h"
#include "qdebug.h"
#include <thread>
#include <QThread>

using namespace mavsdk;

UAVManager::UAVManager(QObject *parent)
    : QObject(parent),
    connectedStatus(false),
    mavsdk(std::make_unique<Mavsdk>(Mavsdk::Configuration{ComponentType::GroundStation})) {}


UAVManager::~UAVManager() {
    disconnectFromUAV();
}



std::unique_ptr<TelemetryHandler>& UAVManager::getTelemetryHandler() {
    return telemetryHandler;
}




QString connectionResultToString(ConnectionResult result) {
    switch (result) {
    case ConnectionResult::Success:
        return "Connection succeeded.";
    case ConnectionResult::Timeout:
        return "Connection timed out.";
    case ConnectionResult::SocketError:
        return "Socket error occurred.";
    case ConnectionResult::BindError:
        return "Bind error occurred.";
    case ConnectionResult::SocketConnectionError:
        return "Socket connection error.";
    case ConnectionResult::ConnectionError:
        return "General connection error.";
    case ConnectionResult::NotImplemented:
        return "Connection type not implemented.";
    case ConnectionResult::SystemNotConnected:
        return "No system is connected.";
    case ConnectionResult::SystemBusy:
        return "System is busy.";
    case ConnectionResult::CommandDenied:
        return "Command denied.";
    case ConnectionResult::DestinationIpUnknown:
        return "Destination IP is unknown.";
    case ConnectionResult::ConnectionsExhausted:
        return "Connections are exhausted.";
    case ConnectionResult::ConnectionUrlInvalid:
        return "Connection URL is invalid.";
    case ConnectionResult::BaudrateUnknown:
        return "Baudrate is unknown.";
    default:
        return "Unknown connection result.";
    }
}



void UAVManager::connectToUAV(const QString &portName, const QString & baudRate) {
    if (connectedStatus) {
        emit connected();
        return;
    }


    if (portName == "Simulation") {
        // Simülasyon seçildiğinde
        connectionString = "udp://:"+ baudRate;
    } else {
        // Gerçek cihaz bağlantısı seçildiğinde
        connectionString = "serial://" + portName + ":" + baudRate;
    }




    auto [connectionResult, tempHandle] = mavsdk->add_any_connection_with_handle(connectionString.toStdString());
    if (connectionResult != ConnectionResult::Success) {
        qDebug() << "Connection failed! Error message:"
                 << connectionResultToString(connectionResult);
        return;
    }
    myConnectionHandle = tempHandle;


    // Cihazın bağlantıya hazır olup olmadığını kontrol et
    bool isConnected = false;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    while (!isConnected && std::chrono::steady_clock::now() - startTime < std::chrono::seconds(5)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!mavsdk->systems().empty() && mavsdk->systems().at(0)->is_connected()) {
            isConnected = true;
        }
    }

    // Eğer sistem bağlanmadıysa, hata mesajı göster
    if (!isConnected) {
        qDebug() <<"Bağlantı kurulamadı, UAV bulunamadı.";
        mavsdk->remove_connection(myConnectionHandle);
        return;
    }

    // Sistem ve plugin'leri başlat
    system = mavsdk->systems().at(0);
    if (!system) {
        qDebug() <<"Sistem başlatılamadı.";
        mavsdk->remove_connection(myConnectionHandle);
        system.reset();

        return;
    }

    // Telemetri ve Action plugin'lerini başlat
    action = std::make_shared<mavsdk::Action>(system);
    if (!action) {
        qDebug() <<"Action plugin'i başlatılamadı.";
        mavsdk->remove_connection(myConnectionHandle);
        action.reset();
        system.reset();


        return;
    }

    telemetry = std::make_shared<mavsdk::Telemetry>(system);
    if (!telemetry) {
        qDebug() <<"Telemetry plugin'i başlatılamadı.";
        mavsdk->remove_connection(myConnectionHandle);
        telemetry.reset();
        system.reset();
        action.reset();


        return;
    }


    telemetryHandler = std::make_unique<TelemetryHandler>(telemetry); // Constructor ile aktar



    QThread *telemetryThread = QThread::create([this]() {
        telemetryHandler->start();  // TelemetryHandler'ın başlatılması
        QMetaObject::invokeMethod(this, "onTelemetryStarted", Qt::QueuedConnection);
    });
    telemetryThread->start(); // Thread'i başlatın




    mission = std::make_shared<mavsdk::Mission>(system);

    connectedStatus = true;
    emit connected();
}

void UAVManager::disconnectFromUAV() {
    if (connectedStatus) {
        connectedStatus = false;
        qDebug() <<"disconneect";

        mavsdk->remove_connection(myConnectionHandle);

        action.reset();
        system.reset();
        telemetry.reset();
        telemetryHandler.reset();
        emit disconnected();
    }
}


void UAVManager::arm() {
    if (!action) {
        qDebug() << "Action nesnesi henüz oluşturulmadı.";
        return;
    }

    auto result = action->arm();
    if (result != mavsdk::Action::Result::Success) {
        qDebug() << "UAV arm edilemedi. Hata:" ;
        return;
    }

    qDebug() << "UAV başarıyla arm edildi.";




}

void UAVManager::takeoff(qint32 takeoff_height) {
    if (!action) {
        qDebug() << "Action nesnesi henüz oluşturulmadı.";
        return;
    }

    // Kalkış yüksekliği ayarlanıyor
    auto result = action->set_takeoff_altitude(static_cast<float>(takeoff_height));
    if (result != mavsdk::Action::Result::Success) {
        qDebug() << "Kalkış yüksekliği ayarlanamadı. Hata:" ;
        return;
    }

    qDebug() << "Kalkış yüksekliği " << takeoff_height << " metre olarak ayarlandı.";

    result = action->takeoff();
    if (result != mavsdk::Action::Result::Success) {
        qDebug() << "Takeoff işlemi başarısız. Hata:" ;
    } else {
        qDebug() << "Takeoff işlemi başarılı.";
    }
}



void UAVManager::sendCoordinatesToUAV(double latitude, double longitude, double altitude, double speed, double yaw)
{
    if (!mission) {
        qDebug() << "Mission plugin'i başlatılamadı!";
        return;
    }


    if (action) {
        auto result = action->set_current_speed((float) speed); // 10 m/s olarak ayarla
        if (result == mavsdk::Action::Result::Success) {
            qDebug() << "UAV hızı başarıyla değiştirildi!";
        } else {
            qDebug() << "Hız değiştirilemedi, hata kodu:" << static_cast<int>(result);
        }
    }

    Mission::MissionItem mission_item;
    mission_item.latitude_deg = latitude;
    mission_item.longitude_deg = longitude;
    mission_item.relative_altitude_m = (float)altitude;
    mission_item.speed_m_s = (float)speed;
    mission_item.is_fly_through = true;
    mission_item.acceptance_radius_m = (float)1.0;
    mission_item.yaw_deg = (float)yaw;



    std::vector<Mission::MissionItem> mission_items;
    mission_items.push_back(mission_item);

    Mission::MissionPlan mission_plan;
    mission_plan.mission_items = mission_items;

    // Yeni bir thread oluşturarak görev yükleme ve başlatma işlemlerini yap
    QThread *missionThread = QThread::create([this, mission_plan]() {
        auto result = mission->upload_mission(mission_plan);
        if (result != mavsdk::Mission::Result::Success) {
            qDebug() << "Görev yükleme hatası: " << static_cast<int>(result);
            return;
        }

        qDebug() << "Görev başarıyla yüklendi!";

        result = mission->start_mission();
        if (result != mavsdk::Mission::Result::Success) {
            qDebug() << "Görev başlatma hatası: " << static_cast<int>(result);
            return;
        }

        qDebug() << "Görev başlatıldı!";
    });

    missionThread->start();
}









bool UAVManager::isConnected() const {
    return connectedStatus;
}

bool UAVManager::areComponentsReady() const {
    return (action != nullptr && telemetry != nullptr);
}

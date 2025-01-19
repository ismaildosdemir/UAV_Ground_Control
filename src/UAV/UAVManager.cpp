#include "UAVManager.h"
#include "qdebug.h"
#include <thread>
#include <QThread>

using namespace mavsdk;

UAVManager::UAVManager(QObject *parent)
    : QObject(parent),
    connectedStatus(false),
    mavsdk(std::make_unique<Mavsdk>(Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation})) {}

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


void UAVManager::takeoff(const qint32 takeoff_height) {
    if (!action) {
        qDebug() << "Action nesnesi henüz oluşturulmadı.";
        return;
    }

    auto result = action->arm();
    if (result != mavsdk::Action::Result::Success) {
        // qDebug() << "UAV arm edilemedi. Hata:" << mavsdk::action_result_str(result).c_str();
        return;
    }

    result = action->takeoff();
    if (result != mavsdk::Action::Result::Success) {
        // qDebug() << "Takeoff işlemi başarısız. Hata:" << mavsdk::action_result_str(result).c_str();
    } else {
        qDebug() << "Takeoff işlemi başarılı.";
    }
}

















bool UAVManager::isConnected() const {
    return connectedStatus;
}

bool UAVManager::areComponentsReady() const {
    return (action != nullptr && telemetry != nullptr);
}

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


void UAVManager::connectToUAV(const QString &portName, const QString &baudRate) {
    if (connectedStatus) {
        emit connected();
        logger->log("Already connected to UAV.", INFO); // Log kaydı ekledik
        return;
    }

    if (portName == "Simulation") {
        connectionString = "udp://:"+ baudRate;
    } else {
        connectionString = "serial://" + portName + ":" + baudRate;
    }

    auto [connectionResult, tempHandle] = mavsdk->add_any_connection_with_handle(connectionString.toStdString());
    if (connectionResult != ConnectionResult::Success) {
        logger->log("Connection failed! Error message: " + connectionResultToString(connectionResult), ERROR);
        return;
    }
    myConnectionHandle = tempHandle;

    bool isConnected = false;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
    while (!isConnected && std::chrono::steady_clock::now() - startTime < std::chrono::seconds(5)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!mavsdk->systems().empty() && mavsdk->systems().at(0)->is_connected()) {
            isConnected = true;
        }
    }

    if (!isConnected) {
        logger->log("Failed to connect, UAV not found.", ERROR);
        mavsdk->remove_connection(myConnectionHandle);
        return;
    }

    system = mavsdk->systems().at(0);
    if (!system) {
        logger->log("Failed to initialize system.", ERROR);
        mavsdk->remove_connection(myConnectionHandle);
        system.reset();
        return;
    }

    action = std::make_shared<mavsdk::Action>(system);
    if (!action) {
        logger->log("Failed to initialize Action plugin.", ERROR);
        mavsdk->remove_connection(myConnectionHandle);
        action.reset();
        system.reset();
        return;
    }

    telemetry = std::make_shared<mavsdk::Telemetry>(system);
    if (!telemetry) {
        logger->log("Failed to initialize Telemetry plugin.", ERROR);
        mavsdk->remove_connection(myConnectionHandle);
        telemetry.reset();
        system.reset();
        action.reset();
        return;
    }

    telemetryHandler = std::make_unique<TelemetryHandler>(telemetry);

    QThread *telemetryThread = QThread::create([this]() {
        telemetryHandler->start();
    });
    telemetryThread->start();

    mission = std::make_shared<mavsdk::Mission>(system);

    connectedStatus = true;
    emit connected();
    logger->log("Successfully connected to UAV.", INFO);  // Log kaydı ekledik
}


void UAVManager::disconnectFromUAV() {
    if (connectedStatus) {
        connectedStatus = false;
        logger->log("Disconnecting from UAV.", INFO);  // Log kaydı ekledik

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
        logger->log("Action object not created yet.", ERROR);
        return;
    }

    auto result = action->arm();
    if (result != mavsdk::Action::Result::Success) {
        logger->log("Failed to arm UAV.", ERROR);
        return;
    }

    logger->log("UAV successfully armed.", INFO);
}

void UAVManager::takeoff(qint32 takeoff_height) {
    if (!action) {
        logger->log("Action object not created yet.", ERROR);
        return;
    }

    auto result = action->set_takeoff_altitude(static_cast<float>(takeoff_height));
    if (result != mavsdk::Action::Result::Success) {
        logger->log("Failed to set takeoff altitude.", ERROR);
        return;
    }

    logger->log("Takeoff altitude set to " + QString::number(takeoff_height) + " meters.", INFO);

    result = action->takeoff();
    if (result != mavsdk::Action::Result::Success) {
        logger->log("Takeoff failed.", ERROR);
    } else {
        logger->log("Takeoff successful.", INFO);
    }
}


void UAVManager::sendCoordinatesToUAV(double latitude, double longitude, double altitude, double speed, double yaw)
{
    if (!mission) {
        logger->log("Mission plugin not initialized.", ERROR);
        return;
    }

    if (action) {
        auto result = action->set_current_speed((float) speed);
        if (result == mavsdk::Action::Result::Success) {
            logger->log("UAV speed successfully changed.", INFO);
        } else {
            logger->log("Failed to change speed, error code: " + QString::number(static_cast<int>(result)), ERROR);
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

    QThread *missionThread = QThread::create([this, mission_plan]() {
        auto result = mission->upload_mission(mission_plan);
        if (result != mavsdk::Mission::Result::Success) {
            logger->log("Mission upload failed, error code: " + QString::number(static_cast<int>(result)), ERROR);
            return;
        }

        logger->log("Mission successfully uploaded.", INFO);

        result = mission->start_mission();
        if (result != mavsdk::Mission::Result::Success) {
            logger->log("Failed to start mission, error code: " + QString::number(static_cast<int>(result)), ERROR);
            return;
        }

        logger->log("Mission started.", INFO);
    });

    missionThread->start();
}



bool UAVManager::isConnected() const {
    return connectedStatus;
}

bool UAVManager::areComponentsReady() const {
    return (action != nullptr && telemetry != nullptr);
}

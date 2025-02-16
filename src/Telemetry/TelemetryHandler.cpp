#include "TelemetryHandler.h"
#include "qdebug.h"
#include <cmath>

// Constructor
TelemetryHandler::TelemetryHandler(std::shared_ptr<mavsdk::Telemetry> telemetry, QObject *parent)
    : QObject(parent), telemetry(std::move(telemetry)) {}

// Destructor
TelemetryHandler::~TelemetryHandler() {}

// Telemetry verilerini başlat
void TelemetryHandler::start() {

    if (!telemetry) {
        qDebug() << "Telemetry shared pointer geçersiz!";
        return;
    }

    subscribePosition();
    subscribeAttitude();
    subscribeFixedwingMetrics();
    subscribeFlightMode();
    subscribeGpsInfo();
    subscribeBattery();
    subscribeArmed();
    subscribeTotalSpeed();
    subscribeHealth(); // Sağlık durumu aboneliği

}

// Getter fonksiyonları
mavsdk::Telemetry::Position TelemetryHandler::getPosition() const {
    return position;
}

mavsdk::Telemetry::EulerAngle TelemetryHandler::getAttitude() const {
    return attitude;
}

mavsdk::Telemetry::FixedwingMetrics TelemetryHandler::getFixedwingMetrics() const {
    return fixedwingMetrics;
}

QString TelemetryHandler::getFlightMode() const {
    return flightMode;
}

mavsdk::Telemetry::GpsInfo TelemetryHandler::getGpsInfo() const {
    return gpsInfo;
}

mavsdk::Telemetry::Battery TelemetryHandler::getBattery() const {
    return battery;
}

bool TelemetryHandler::isArmed() const {
    return armed;
}

double TelemetryHandler::getTotalSpeed() const {
    return totalSpeed;
}

mavsdk::Telemetry::Health TelemetryHandler::getHealth() const {
    return health;
}

// Telemetry verileri için abonelik fonksiyonları
void TelemetryHandler::subscribePosition() {
    telemetry->subscribe_position([this](const mavsdk::Telemetry::Position &pos) {
        position = pos;
        //qDebug() << "Position telemetry updated!";

        emit telemetryDataUpdated();
    });
}

void TelemetryHandler::subscribeAttitude() {
    telemetry->subscribe_attitude_euler([this](const mavsdk::Telemetry::EulerAngle &att) {
        attitude = att;
        emit telemetryDataUpdated();
    });
}

void TelemetryHandler::subscribeFixedwingMetrics() {
    telemetry->subscribe_fixedwing_metrics([this](const mavsdk::Telemetry::FixedwingMetrics &metrics) {
        fixedwingMetrics = metrics;
        emit telemetryDataUpdated();
    });
}

void TelemetryHandler::subscribeFlightMode() {
    telemetry->subscribe_flight_mode([this](mavsdk::Telemetry::FlightMode mode) {
        flightMode = flightModeToString(mode);
        emit telemetryDataUpdated();
    });
}

void TelemetryHandler::subscribeGpsInfo() {
    telemetry->subscribe_gps_info([this](const mavsdk::Telemetry::GpsInfo &info) {
        gpsInfo = info;
        emit telemetryDataUpdated();
    });
}

void TelemetryHandler::subscribeBattery() {
    telemetry->subscribe_battery([this](const mavsdk::Telemetry::Battery &batt) {
        battery = batt;
        emit telemetryDataUpdated();
    });
}

void TelemetryHandler::subscribeArmed() {
    telemetry->subscribe_armed([this](bool arm_status) {
        armed = arm_status;
        emit telemetryDataUpdated();
    });
}

void TelemetryHandler::subscribeTotalSpeed() {
    telemetry->subscribe_velocity_ned([this](const mavsdk::Telemetry::VelocityNed &velocityNed) {
        totalSpeed = std::sqrt(
            velocityNed.north_m_s * velocityNed.north_m_s +
            velocityNed.east_m_s * velocityNed.east_m_s +
            velocityNed.down_m_s * velocityNed.down_m_s
            );
        emit telemetryDataUpdated();
    });
}


// Sağlık durumu abonelik fonksiyonu
void TelemetryHandler::subscribeHealth() {
    telemetry->subscribe_health([this](const mavsdk::Telemetry::Health& healthData) {
        health = healthData;

        // Sağlık durumu bilgilerini yazdırma
        //qDebug() << "Gyro Calibrated: " << (healthData.is_gyrometer_calibration_ok ? "Yes" : "No");
      //  qDebug() << "Accel Calibrated: " << (healthData.is_accelerometer_calibration_ok ? "Yes" : "No");
     //   qDebug() << "Mag Calibrated: " << (healthData.is_magnetometer_calibration_ok ? "Yes" : "No");
     //   qDebug() << "Local Position OK: " << (healthData.is_local_position_ok ? "Yes" : "No");
     //   qDebug() << "Global Position OK: " << (healthData.is_global_position_ok ? "Yes" : "No");
     //   qDebug() << "Home Position Initialized: " << (healthData.is_home_position_ok ? "Yes" : "No");
     //   qDebug() << "Armable: " << (healthData.is_armable ? "Yes" : "No");

        emit telemetryDataUpdated();
    });
}




QString TelemetryHandler::flightModeToString(mavsdk::Telemetry::FlightMode mode) {
    switch (mode) {
    case mavsdk::Telemetry::FlightMode::Unknown: return "Unknown";
    case mavsdk::Telemetry::FlightMode::Ready: return "Ready";
    case mavsdk::Telemetry::FlightMode::Takeoff: return "Takeoff";
    case mavsdk::Telemetry::FlightMode::Hold: return "Hold";
    case mavsdk::Telemetry::FlightMode::Mission: return "Mission";
    case mavsdk::Telemetry::FlightMode::ReturnToLaunch: return "Return to Launch";
    case mavsdk::Telemetry::FlightMode::Land: return "Land";
    case mavsdk::Telemetry::FlightMode::Offboard: return "Offboard";
    case mavsdk::Telemetry::FlightMode::FollowMe: return "Follow Me";
    case mavsdk::Telemetry::FlightMode::Manual: return "Manual";
    case mavsdk::Telemetry::FlightMode::Altctl: return "Altitude Control";
    case mavsdk::Telemetry::FlightMode::Posctl: return "Position Control";
    case mavsdk::Telemetry::FlightMode::Acro: return "Acro";
    case mavsdk::Telemetry::FlightMode::Stabilized: return "Stabilized";
    case mavsdk::Telemetry::FlightMode::Rattitude: return "Rattitude";
    default: return "Unknown Flight Mode";
    }
}

QString gpsFixTypeToString(const mavsdk::Telemetry::GpsInfo& value) {
    switch(value.fix_type) {
    case mavsdk::Telemetry::FixType::NoGps: return "NoGps";
    case mavsdk::Telemetry::FixType::NoFix: return "NoFix";
    case mavsdk::Telemetry::FixType::Fix2D: return "Fix2D";
    case mavsdk::Telemetry::FixType::Fix3D: return "Fix3D";
    case mavsdk::Telemetry::FixType::FixDgps: return "FixDgps";
    case mavsdk::Telemetry::FixType::RtkFloat: return "RtkFloat";
    case mavsdk::Telemetry::FixType::RtkFixed: return "RtkFixed";
    default: return "Unknown";
    }
}

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

mavsdk::Telemetry::FlightMode TelemetryHandler::getFlightMode() const {
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

// Telemetry verileri için abonelik fonksiyonları
void TelemetryHandler::subscribePosition() {
    telemetry->subscribe_position([this](const mavsdk::Telemetry::Position &pos) {
        position = pos;
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
        flightMode = mode;
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

void TelemetryHandler::calculateTotalSpeed(const mavsdk::Telemetry::VelocityNed &velocityNed) {
    totalSpeed = std::sqrt(
        velocityNed.north_m_s * velocityNed.north_m_s +
        velocityNed.east_m_s * velocityNed.east_m_s +
        velocityNed.down_m_s * velocityNed.down_m_s
        );
    emit telemetryDataUpdated();
}

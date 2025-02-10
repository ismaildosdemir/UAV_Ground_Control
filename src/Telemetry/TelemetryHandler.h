#ifndef TELEMETRYHANDLER_H
#define TELEMETRYHANDLER_H

#include <QObject>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <memory>

// TelemetryHandler sınıfı
class TelemetryHandler : public QObject {
    Q_OBJECT

public:
    // Constructor ve Destructor
    explicit TelemetryHandler(std::shared_ptr<mavsdk::Telemetry> telemetry, QObject *parent = nullptr);
    ~TelemetryHandler();

    // Telemetry verilerini başlat
    void start();

    // Getter fonksiyonları
    mavsdk::Telemetry::Position getPosition() const;
    mavsdk::Telemetry::EulerAngle getAttitude() const;
    mavsdk::Telemetry::FixedwingMetrics getFixedwingMetrics() const;
    QString getFlightMode() const;
    mavsdk::Telemetry::GpsInfo getGpsInfo() const;
    mavsdk::Telemetry::Battery getBattery() const;
    bool isArmed() const;
    double getTotalSpeed() const;
    mavsdk::Telemetry::Health getHealth() const;  // Sağlık durumu getter'ı

signals:
    // Sinyaller
    void telemetryDataUpdated();

private:
    // Telemetry referansı
    std::shared_ptr<mavsdk::Telemetry> telemetry;

    // Veriler
    mavsdk::Telemetry::Position position;
    mavsdk::Telemetry::EulerAngle attitude;
    mavsdk::Telemetry::FixedwingMetrics fixedwingMetrics;
    QString flightMode;
    mavsdk::Telemetry::GpsInfo gpsInfo;
    mavsdk::Telemetry::Battery battery;
    bool armed = false;
    double totalSpeed = 0.0;

    // Telemetry verilerini güncelleyen yardımcı fonksiyonlar
    void subscribePosition();
    void subscribeAttitude();
    void subscribeFixedwingMetrics();
    void subscribeFlightMode();
    void subscribeGpsInfo();
    void subscribeBattery();
    void subscribeArmed();
    void subscribeTotalSpeed();
    void subscribeHealth();

    QString flightModeToString(mavsdk::Telemetry::FlightMode mode);
    mavsdk::Telemetry::Health health;  // Sağlık durumu verisini tutacak üyeyi ekledik

};

#endif // TELEMETRYHANDLER_H

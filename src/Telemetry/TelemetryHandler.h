#ifndef TELEMETRYHANDLER_H
#define TELEMETRYHANDLER_H

#include <QObject>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <memory>
#include <mavsdk/log_callback.h>

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
    mavsdk::Telemetry::Heading getHeading() const;
    mavsdk::Telemetry::Position getPosition() const;
    mavsdk::Telemetry::EulerAngle getAttitude() const;
    mavsdk::Telemetry::FixedwingMetrics getFixedwingMetrics() const;
    QString getFlightMode() const;
    mavsdk::Telemetry::GpsInfo getGpsInfo() const;
    mavsdk::Telemetry::Battery getBattery() const;
    bool isArmed() const;
    double getTotalSpeed() const;
    mavsdk::Telemetry::Health getHealth() const;  // Sağlık durumu getter'ı
    mavsdk::Telemetry::RcStatus getRcStatus() const;
    std::pair<QString, mavsdk::log::Level> getLastLog() const; // Tek fonksiyonla hem mesaj hem seviyeyi almak

signals:
    // Sinyaller
    void telemetryDataUpdated();
    void UavLogDataUpdated();

private:
    // Telemetry referansı
    std::shared_ptr<mavsdk::Telemetry> telemetry;

    // Veriler
    mavsdk::Telemetry::Heading heading;
    mavsdk::Telemetry::Position position;
    mavsdk::Telemetry::EulerAngle attitude;
    mavsdk::Telemetry::FixedwingMetrics fixedwingMetrics;
    QString flightMode;
    mavsdk::Telemetry::GpsInfo gpsInfo;
    mavsdk::Telemetry::Battery battery;
    mavsdk::Telemetry::Health health;  // Sağlık durumu verisini tutacak üyeyi ekledik
    mavsdk::Telemetry::RcStatus rcStatus;

    bool armed = false;
    double totalSpeed = 0.0;

    QString lastLogMessage;  // Son log mesajını saklamak için
    mavsdk::log::Level lastLogLevel; // Son log seviyesini saklamak için

    // Telemetry verilerini güncelleyen yardımcı fonksiyonlar
    void subscribePosition();
    void subscribeheading();
    void subscribeAttitude();
    void subscribeFixedwingMetrics();
    void subscribeFlightMode();
    void subscribeGpsInfo();
    void subscribeBattery();
    void subscribeArmed();
    void subscribeTotalSpeed();
    void subscribeHealth();
    void subscribeConnnectionState();
    void subscribeLog();

    QString flightModeToString(mavsdk::Telemetry::FlightMode mode);

};

#endif // TELEMETRYHANDLER_H

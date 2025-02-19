#ifndef UAVMANAGER_H
#define UAVMANAGER_H

#include "src/Telemetry/TelemetryHandler.h"
#include "src/Utils/Logger.h"
#include <QObject>
#include <memory>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/mission_raw/mission_raw.h>
#include <mavsdk/plugins/mission/mission.h>

enum class FlightCommand { TakeOff, Land, ReturnToHome };

class UAVManager : public QObject {
    Q_OBJECT

public:
    void extracted();
    explicit UAVManager(QObject *parent = nullptr);
    ~UAVManager();

    // Bağlantı İşlevleri
    void connectToUAV(const QString &portName, const QString & baudRate);
    void disconnectFromUAV();
    bool isConnected() const;
    bool areComponentsReady() const;
    void takeoff(const qint32 takeoff_height);
    void arm();

    std::unique_ptr<TelemetryHandler>& getTelemetryHandler(); // Sadece prototip
    void sendCoordinatesToUAV(double latitude, double longitude, double altitude, double speed, double yaw);


signals:
    void connected();
    void disconnected();

private:
    // ConnectionHandle ve Handle şablonunun tanımlanması
    using ConnectionHandle = mavsdk::Handle<>;  // Handle<> türüne dayanan ConnectionHandle
    // ConnectionHandle türünde bir değişken oluşturma
    ConnectionHandle myConnectionHandle;
    std::unique_ptr<TelemetryHandler> telemetryHandler; // Üye değişken
    std::shared_ptr<mavsdk::Mission> mission;
    bool connectedStatus = false; // Varsayılan olarak bağlantı durumu yanlış
    QString connectionString;
    std::unique_ptr<mavsdk::Mavsdk> mavsdk;
    std::shared_ptr<mavsdk::System> system;
    std::shared_ptr<mavsdk::Action> action;
    std::shared_ptr<mavsdk::Telemetry> telemetry;
    Logger *logger;  // Logger sınıfının bir örneği
};

#endif // UAVMANAGER_H

#ifndef UAVMANAGER_H
#define UAVMANAGER_H

#include "src/Telemetry/TelemetryHandler.h"
#include <QObject>
#include <memory>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/action/action.h>

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
    std::unique_ptr<TelemetryHandler>& getTelemetryHandler(); // Sadece prototip


signals:
    void connected();
    void disconnected();

private:
    // ConnectionHandle ve Handle şablonunun tanımlanması
    using ConnectionHandle = mavsdk::Handle<>;  // Handle<> türüne dayanan ConnectionHandle
    // ConnectionHandle türünde bir değişken oluşturma
    ConnectionHandle myConnectionHandle;
    std::unique_ptr<TelemetryHandler> telemetryHandler; // Üye değişken




    bool connectedStatus = false; // Varsayılan olarak bağlantı durumu yanlış
    QString connectionString;
    std::unique_ptr<mavsdk::Mavsdk> mavsdk;
    std::shared_ptr<mavsdk::System> system;
    std::shared_ptr<mavsdk::Action> action;
    std::shared_ptr<mavsdk::Telemetry> telemetry;

};

#endif // UAVMANAGER_H

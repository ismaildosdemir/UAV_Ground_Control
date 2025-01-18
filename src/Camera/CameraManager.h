#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QCamera>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QMediaCaptureSession>

class CameraManager : public QObject
{
    Q_OBJECT

public:
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();

    void connectToCamera(const QString &cameraName);
    void disconnectCamera();
    QStringList availableCameras() const;
    QMediaCaptureSession* getCaptureSession() const;
    QCamera *camera = nullptr;
    bool isCameraConnected() const;

signals:
    void cameraStarted(const QString &cameraName); // Kamera açıldığında sinyal
    void cameraStopped(); // Kamera kapatıldığında sinyal


private:
    //QCamera *camera = nullptr;
    QMediaCaptureSession *captureSession = nullptr;
};

#endif // CAMERAMANAGER_H

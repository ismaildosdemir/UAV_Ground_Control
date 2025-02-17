#include "CameraManager.h"
#include <QDebug>
#include "src/Utils/Logger.h"

CameraManager::CameraManager(QObject *parent)
    : QObject(parent),
    camera(nullptr),
    captureSession(new QMediaCaptureSession(this))
{
}

CameraManager::~CameraManager()
{
    disconnectCamera();
    delete captureSession;
}

void CameraManager::connectToCamera(const QString &cameraName)
{
    if (cameraName.isEmpty()) {
        Logger::instance().log("Kamera adı boş!", ERROR);  // Log: Kamera adı boş
        qDebug() << "Kamera adı boş!";
        return;
    }

    disconnectCamera(); // Önceki kamerayı temizle

    const auto cameraDevices = QMediaDevices::videoInputs();
    bool foundCamera = false;

    for (const QCameraDevice &device : cameraDevices) {
        if (device.description() == cameraName) {
            camera = new QCamera(device, this);
            foundCamera = true;
            break;
        }
    }

    if (!foundCamera) {
        QString errorMsg = "Kamera bulunamadı: " + cameraName;
        Logger::instance().log(errorMsg, ERROR);  // Log: Kamera bulunamadı
        qDebug() << errorMsg;
        return;
    }

    captureSession->setCamera(camera);
    camera->start();

    if (camera && camera->isActive()) { // Kamera başarılı şekilde başladı
        QString successMsg = "Kamera başarıyla bağlandı: " + cameraName;
        Logger::instance().log(successMsg, INFO);  // Log: Kamera başarılı şekilde bağlandı
        emit cameraStarted(cameraName); // Kamera açıldığını bildir
        qDebug() << successMsg;
    } else {
        Logger::instance().log("Kamera başlatılamadı!", ERROR);  // Log: Kamera başlatılamadı
        qDebug() << "Kamera başlatılamadı!";
        disconnectCamera();
    }
}

void CameraManager::disconnectCamera()
{
    if (camera) {
        Logger::instance().log("Kamera durduruluyor...", INFO);  // Log: Kamera durduruluyor
        qDebug() << "Kamera durduruluyor...";
        camera->stop(); // Kamerayı durdur

        emit cameraStopped(); // Kamera kapandığını bildir
        Logger::instance().log("Kamera durduruldu.", INFO);  // Log: Kamera durduruldu
        qDebug() << "Kamera durduruldu";

        delete camera; // Kamera nesnesini sil
        camera = nullptr;
    }
}

QStringList CameraManager::availableCameras() const
{
    QStringList cameraList;
    const auto cameraDevices = QMediaDevices::videoInputs();
    for (const QCameraDevice &device : cameraDevices) {
        cameraList << device.description();
    }
    return cameraList;
}

QMediaCaptureSession* CameraManager::getCaptureSession() const
{
    return captureSession;
}

bool CameraManager::isCameraConnected() const
{
    return camera != nullptr && camera->isActive();
}

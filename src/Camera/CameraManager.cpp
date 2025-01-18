#include "CameraManager.h"
#include <QDebug>

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
        qDebug() << "Kamera bulunamadı: " << cameraName;
        return;
    }

    captureSession->setCamera(camera);
    camera->start();

    if (camera && camera->isActive()) { // Kamera başarılı şekilde başladı
        qDebug() << "Kamera başarıyla bağlandı: " << cameraName;
        emit cameraStarted(cameraName); // Kamera açıldığını bildir
    } else {
        qDebug() << "Kamera başlatılamadı!";
        disconnectCamera();
    }
}

void CameraManager::disconnectCamera()
{
    if (camera) {

        qDebug() << "Kamera durduruluyor...";
        camera->stop(); // Kamerayı durdur

        emit cameraStopped(); // Kamera kapandığını bildir

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qlabel.h"
#include "src/Camera/CameraManager.h"
#include "src/UAV/UAVManager.h"
#include "src/Utils/Logger.h"
#include <QMainWindow>
#include <QVideoWidget>
#include <QCamera>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QMediaCaptureSession* getCaptureSession() const { return captureSession; }


private:
    Ui::MainWindow *ui;
    UAVManager *uavManager; // UAVManager nesnesi
    QString connectionString;
    void setupConnections(); // Signal-slot bağlantıları için bir yardımcı fonksiyon
    void listSerialPortsAndConnections();
    void showTime();
    Logger *logger;  // Logger sınıfının bir örneği
    // void updateTelemetryData();
    void onUAVConnected();
    CameraManager *cameraManager;
    void cameraConnectPushButton_clicked();
    QVideoWidget *videoWidget;
    QMediaCaptureSession* captureSession;
    QCamera *camera;
    MainWindow* mainWindowPointer; // MainWindow işaretçisi
    void setLabel(QLabel* label, bool condition, const QString& trueText, const QString& falseText, const QString& fontFamily, int fontSize, int fontWeight);


private slots:
    void updateUAVPosition(double latitude, double longitude);


public slots:
    void updateTelemetryData();
    void updateCoordinates(double latitude, double longitude);
};

#endif // MAINWINDOW_H

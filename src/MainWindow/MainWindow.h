#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "src/UAV/UAVManager.h"
#include "src/Utils/Logger.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    UAVManager *uavManager; // UAVManager nesnesi
    QString connectionString;
    void setupConnections(); // Signal-slot bağlantıları için bir yardımcı fonksiyon
    void listSerialPortsAndConnections();
    void showTime();
    Logger *logger;  // Logger sınıfının bir örneği
    void updateTelemetryData();
    void onUAVConnected();

};

#endif // MAINWINDOW_H

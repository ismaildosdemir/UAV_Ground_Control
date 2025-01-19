#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QSerialPortInfo>
#include <QDir>
#include <QMediaDevices>
#include <QTimer>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include "qboxlayout.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , uavManager(new UAVManager(this))
    , cameraManager(new CameraManager(this))
    , videoWidget(new QVideoWidget(this))
    ,captureSession(new QMediaCaptureSession(this))
{
    // UI öğelerini başlat
    ui->setupUi(this);


    // Logger mesajı
    //Logger::instance().log("MainWindow oluşturuldu.");

    // Signal-slot bağlantıları
    setupConnections();

    // Bağlantı ve baudrate listeleme fonksiyonu çağrısı
    listSerialPortsAndConnections();


    // // Subscribe to MAVSDK log messages
    // mavsdk::log::subscribe([this](mavsdk::log::Level level, const std::string& message, const std::string&, int) {
    //     // Print log message to console for debugging
    //     Logger::instance().appendLogMessage(QString::fromStdString(message),ui->plainTextEdit,"",level);
    //     // Return true to prevent default MAVSDK logging to stdout
    //     return true;
    // });


    // UI'deki videoFrame'i ve videoWidget'ı hazırlayın
    QVBoxLayout *layout = new QVBoxLayout(ui->videoFrame);
    layout->addWidget(videoWidget);
    ui->videoFrame->setLayout(layout);
    videoWidget->hide();   // Video alanını gizle


    // Saati hemen göster
    showTime();
    // QTimer ile saati her saniye güncelle
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::showTime);
    timer->start(1000); // 1000 ms = 1 saniye




    ui->armStringLabel->setAlignment(Qt::AlignCenter);
    ui->flightModeLabel->setAlignment(Qt::AlignCenter);




}

MainWindow::~MainWindow()
{
    //Logger::instance().log("MainWindow yok ediliyor.");
    delete ui;
}

void MainWindow::setupConnections()
{

    // "Connect UAV" butonuna tıklama olayını bağla
    connect(ui->connectPushButton, &QPushButton::clicked, this, [=]() {
        if (!uavManager->isConnected()) {
            QString portName = ui->connectionComboBox->currentText();
            QString baudRate = ui->baundComboBox->currentText();

            // UAVManager üzerinden bağlantıyı başlat
            uavManager->connectToUAV(portName, baudRate);
        }
        else {
            // Bağlantıyı kesme isteği
            uavManager->disconnectFromUAV();
        }
    });

    // Bağlanma sinyalini dinle
    connect(uavManager, &UAVManager::connected, this, [=]() {
        ui->connectPushButton->setText("Disconnect UAV"); // Buton metnini güncelle
        qDebug() << "UAV başarıyla bağlandı!";
    });

    // Bağlantı kesilme sinyalini dinle
    connect(uavManager, &UAVManager::disconnected, this, [=]() {
        ui->connectPushButton->setText("Connect UAV"); // Buton metnini güncelle
        qDebug() << "UAV bağlantısı kesildi.";
    });

    connect(uavManager, &UAVManager::connected, this, &MainWindow::onUAVConnected);

    connect(ui->cameraConnectPushButton, &QPushButton::clicked, this, &MainWindow::cameraConnectPushButton_clicked);

    connect(cameraManager, &CameraManager::cameraStarted, this, [this](const QString &cameraName) {
        ui->cameraConnectPushButton->setText("Disconnect Camera"); // Buton metnini güncelle
        videoWidget->show();   // Yeniden göster
        qDebug() << "Kamera açıldı: " << cameraName;
    });

    connect(cameraManager, &CameraManager::cameraStopped, this, [this]() {
        ui->cameraConnectPushButton->setText("Connect Camera"); // Buton metnini güncelle
        videoWidget->hide();   // Video alanını gizle
        qDebug() << "Kamera kapandı: ";
    });

    connect(ui->refreshPortsPushButton, &QPushButton::clicked, this, &MainWindow::listSerialPortsAndConnections);




    //Logger::instance().log("Signal-slot bağlantıları ayarlandı.");
}

void MainWindow::listSerialPortsAndConnections() {

    // Yeni portları temizle
    ui->connectionComboBox->clear();
    // Bağlantılı olan COM portlarını al
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    // COM portlarını connectionComboBox'a ekle
    for (const QSerialPortInfo &port : ports) {
        // Bağlı port adlarını ekleyelim (örneğin /dev/ttyUSB0)
        ui->connectionComboBox->addItem(port.portName());
    }
    // Açık bağlantılar için simülasyon modunu connectionComboBox'a ekleyelim
    // Örneğin, Simülasyon bağlantısı 'udp://:14550'
    ui->connectionComboBox->addItem("Simulation");



    // Baud rate listesi, örneğin:
    ui->baundComboBox->clear();
    ui->baundComboBox->addItem("9600");
    ui->baundComboBox->addItem("19200");
    ui->baundComboBox->addItem("38400");
    ui->baundComboBox->addItem("57600");
    ui->baundComboBox->addItem("115200");
    ui->baundComboBox->addItem("14550");



    // Kameraları combobox'a yükleme
    ui->cameraComboBox->clear();
    QStringList cameras = cameraManager->availableCameras();
    ui->cameraComboBox->addItems(cameras);

}

void MainWindow::updateTelemetryData() {



    auto& telemetryHandler = uavManager->getTelemetryHandler();


    // Position bilgilerini güncelle
    auto position = telemetryHandler->getPosition();
    ui->altitudeLabel->setText(QString::number(position.absolute_altitude_m, 'f', 2));

    /// position'dan gelen kordinat bilgileri ile haritayı güncelle




    // Battery bilgilerini güncelle
    auto battery = telemetryHandler->getBattery();
    ui->batteryLabel->setText(QString::number(battery.remaining_percent, 'f', 1) + "%");

    // Total Speed bilgilerini güncelle
    ui->speedLabel->setText(QString::number(telemetryHandler->getTotalSpeed(), 'f', 2) + " m/s");




    auto attitude = telemetryHandler->getAttitude();
    auto fixedwingMetrics = telemetryHandler->getFixedwingMetrics();

    ui->AttitudeGraphicsView->setPitch(attitude.pitch_deg);
    ui->AttitudeGraphicsView->setRoll(attitude.roll_deg);
    ui->AttitudeGraphicsView->setAirspeed(fixedwingMetrics.airspeed_m_s);
    ui->AttitudeGraphicsView->setAirspeedSel(fixedwingMetrics.airspeed_m_s);
    ui->AttitudeGraphicsView->setAltitude(position.relative_altitude_m);
    ui->AttitudeGraphicsView->setAltitudeSel(position.relative_altitude_m);
    ui->AttitudeGraphicsView->setClimbRate(fixedwingMetrics.climb_rate_m_s);
    ui->AttitudeGraphicsView->setHeading(attitude.yaw_deg);
    ui->AttitudeGraphicsView->setHeadingSel(attitude.yaw_deg);
    ui->AttitudeGraphicsView->redraw();


    auto gps = telemetryHandler->getGpsInfo();
    ui->satCountLabel->setText(QString::number(gps.num_satellites));
    std::stringstream ss;
    ss << gps.fix_type;
    ui->gpsLabel->setText(QString::fromStdString(ss.str()));




    QString mode = telemetryHandler->getFlightMode();
    ui->flightModeLabel->setText(mode);


    ui->armStringLabel->setText(telemetryHandler->isArmed() ? "ARM" : "DISARM");












}

void MainWindow::onUAVConnected() {

    // UAV bağlandıktan sonra telemetri sinyalini güncelleme fonksiyonuna bağla
    auto& telemetryHandler = uavManager->getTelemetryHandler();

    if (!telemetryHandler) {
        qDebug() << "TelemetryHandler bulunamadı!";
        return;
    }

    bool connected = connect(telemetryHandler.get(), &TelemetryHandler::telemetryDataUpdated, this, &MainWindow::updateTelemetryData);
    if (!connected) {
        qDebug() << "Sinyal bağlantısı başarısız!";
    } else {
        qDebug() << "Sinyal bağlantısı başarılı!";
    }


    qDebug() << "TelemetryHandler sinyali updateTelemetryData fonksiyonuna bağlandı.";
}

void MainWindow::cameraConnectPushButton_clicked()
{

// camera is connect? if/else
    if (cameraManager->isCameraConnected()) {
    cameraManager->disconnectCamera();

    } else {
        QString cameraName = ui->cameraComboBox->currentText();
        cameraManager->connectToCamera(cameraName);

        // UAVManager'dan captureSession alın
        captureSession = cameraManager->getCaptureSession(); // UAVManager'dan captureSession alın

        // CaptureSession'ı videoWidget ile ilişkilendir
        captureSession->setVideoOutput(videoWidget);
    }
}

void MainWindow::showTime(){
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("hh:mm:ss");
    ui->timeLabel->setText(formattedDateTime);
}

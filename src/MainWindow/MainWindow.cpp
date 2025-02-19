#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QSerialPortInfo>
#include <QDir>
#include <QMediaDevices>
#include <QTimer>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include "qboxlayout.h"
#include <QQmlContext>
#include <QQuickItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , uavManager(new UAVManager(this))
    , cameraManager(new CameraManager(this))
    , videoWidget(new QVideoWidget(this))
    ,captureSession(new QMediaCaptureSession(this))
{
    ui->setupUi(this);
    Logger::instance().log("MainWindow oluşturuldu.");

    setupConnections();
    listSerialPortsAndConnections();

    QVBoxLayout *layout = new QVBoxLayout(ui->videoFrame);
    layout->addWidget(videoWidget);
    ui->videoFrame->setLayout(layout);
    videoWidget->hide();

    showTime();
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::showTime);
    timer->start(1000);

    ui->armStringLabel->setAlignment(Qt::AlignCenter);
    ui->flightModeLabel->setAlignment(Qt::AlignCenter);

    ui->quickWidget->rootContext()->setContextProperty("mapFunction", this);
}

MainWindow::~MainWindow()
{
    Logger::instance().log("MainWindow yok ediliyor.");
    delete ui;
}


void MainWindow::setupConnections()
{
    Logger::instance().log("setupConnections: Bağlantılar kuruluyor...");

    // "Connect UAV" butonuna tıklama olayını bağla
    connect(ui->connectPushButton, &QPushButton::clicked, this, [=]() {
        if (!uavManager->isConnected()) {
            QString portName = ui->connectionComboBox->currentText();
            QString baudRate = ui->baundComboBox->currentText();

            Logger::instance().log("UAV bağlantı isteği gönderildi: Port=" + portName + ", Baud=" + baudRate);
            uavManager->connectToUAV(portName, baudRate);
        }
        else {
            Logger::instance().log("UAV bağlantısı kesme isteği gönderildi.");
            uavManager->disconnectFromUAV();
        }
    });

    // Bağlanma sinyalini dinle
    connect(uavManager, &UAVManager::connected, this, [=]() {
        ui->connectPushButton->setText("Disconnect UAV");
        ui->connectionStatusLabel->setText("CONNECTED");
        ui->connectionStatusLabel->setStyleSheet("color: rgb(34, 177, 76); font: 700 10pt 'Segoe UI';");
        Logger::instance().log("UAV bağlantısı başarılı.");
        qDebug() << "UAV başarıyla bağlandı!";
    });

    // Bağlantı kesilme sinyalini dinle
    connect(uavManager, &UAVManager::disconnected, this, [=]() {
        ui->connectPushButton->setText("Connect UAV");
        ui->connectionStatusLabel->setText("NOT CONNECTED !!!");
        ui->connectionStatusLabel->setStyleSheet("color: rgb(255, 19, 90); font: 700 10pt 'Segoe UI';");
        Logger::instance().log("UAV bağlantısı kesildi.");
        qDebug() << "UAV bağlantısı kesildi.";
    });

    connect(uavManager, &UAVManager::connected, this, &MainWindow::onUAVConnected);

    connect(ui->cameraConnectPushButton, &QPushButton::clicked, this, &MainWindow::cameraConnectPushButton_clicked);

    connect(cameraManager, &CameraManager::cameraStarted, this, [this](const QString &cameraName) {
        ui->cameraConnectPushButton->setText("Disconnect Camera");
        videoWidget->show();
        Logger::instance().log("Kamera açıldı: " + cameraName);
        qDebug() << "Kamera açıldı: " << cameraName;
    });

    connect(cameraManager, &CameraManager::cameraStopped, this, [this]() {
        ui->cameraConnectPushButton->setText("Connect Camera");
        videoWidget->hide();
        Logger::instance().log("Kamera kapandı.");
        qDebug() << "Kamera kapandı.";
    });

    connect(ui->refreshPortsPushButton, &QPushButton::clicked, this, &MainWindow::listSerialPortsAndConnections);

    connect(ui->armPushButton, &QPushButton::clicked, this, [this]() {
        Logger::instance().log("UAV arm isteği gönderildi.");
        uavManager->arm();
    });

    connect(ui->takeoffPushButton, &QPushButton::clicked, this, [this]() {
        int takeoffHeight = ui->altitudeSpinBox->value();
        Logger::instance().log("UAV kalkış isteği gönderildi. Yükseklik: " + QString::number(takeoffHeight));
        uavManager->takeoff(takeoffHeight);
    });

    connect(ui->headTowardsPushButton, &QPushButton::clicked, this, [this]() {
        int takeoffHeight = ui->altitudeSpinBox->value();
        int speed = ui->speedSpinBox->value();
        int yaw = ui->yawSpinBox->value();

        Logger::instance().log("UAV hedefe yönlendirme isteği gönderildi. Yükseklik: " + QString::number(takeoffHeight) +
                               ", Hız: " + QString::number(speed) + ", Yaw: " + QString::number(yaw));

        uavManager->sendCoordinatesToUAV(togoLat, togoLon, takeoffHeight, speed, yaw);
    });

    Logger::instance().log("setupConnections: Tüm bağlantılar başarıyla kuruldu.");
}


void MainWindow::listSerialPortsAndConnections()
{
    Logger::instance().log("Bağlantı noktaları ve baud rate listeleniyor.");
    ui->connectionComboBox->clear();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        ui->connectionComboBox->addItem(port.portName());
    }
    ui->connectionComboBox->addItem("Simulation");
    ui->baundComboBox->clear();
    ui->baundComboBox->addItems({"9600", "19200", "38400", "57600", "115200", "14550"});
    ui->cameraComboBox->clear();
    ui->cameraComboBox->addItems(cameraManager->availableCameras());
}



void MainWindow::updateTelemetryData() {


    auto& telemetryHandler = uavManager->getTelemetryHandler();


    // Position bilgilerini güncelle
    auto position = telemetryHandler->getPosition();
    ui->altitudeLabel->setText(QString::number(position.absolute_altitude_m, 'f', 2));

    auto heading = telemetryHandler->getHeading();

    /// position'dan gelen kordinat bilgileri ile haritayı güncelle
    updateUAVPosition(position.latitude_deg, position.longitude_deg, heading.heading_deg);



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



    // ARM/DISARM Label'ı için özel font ve boyut
    setLabel(ui->armStringLabel, telemetryHandler->isArmed(), "ARM", "DISARM", "Ubuntu", 20, 700);

    // Health verilerini almak
    auto health = telemetryHandler->getHealth();

    // Kalibrasyonlar için font: 700 14pt "Ubuntu"
    setLabel(ui->gyroValueLabel, health.is_gyrometer_calibration_ok, "Calibrated", "Not Calibrated", "Ubuntu", 14, 700);
    setLabel(ui->accelValueLabel, health.is_accelerometer_calibration_ok, "Calibrated", "Not Calibrated", "Ubuntu", 14, 700);
    setLabel(ui->magValueLabel, health.is_magnetometer_calibration_ok, "Calibrated", "Not Calibrated", "Ubuntu", 14, 700);

    // Diğer veriler için font: 700 14pt "Ubuntu"
    setLabel(ui->localPosValueLabel, health.is_local_position_ok, "Good", "Bad", "Ubuntu", 14, 700);
    setLabel(ui->globalPosValueLabel, health.is_global_position_ok, "Good", "Bad", "Ubuntu", 14, 700);
    setLabel(ui->homePosValueLabel, health.is_home_position_ok, "Initialized", "Not Initialized", "Ubuntu", 14, 700);
    setLabel(ui->armableValueLabel, health.is_armable, "Yes", "No", "Ubuntu", 14, 700);


    auto RcStatus = telemetryHandler->getRcStatus();
    ui->signalLabel->setText(QString::number(RcStatus.signal_strength_percent, 'f', 2) + "%" );

}



void MainWindow::setLabel(QLabel* label, bool condition, const QString& trueText, const QString& falseText,
                          const QString& fontFamily, int fontSize, int fontWeight) {
    label->setText(condition ? trueText : falseText);

    // Stil ve font ayarlarını yapıyoruz
    label->setStyleSheet(QString("color: %1; font-family: %2; font-size: %3pt; font-weight: %4;")
                             .arg(condition ? "#33d9b2" : "#ff5252")
                             .arg(fontFamily)
                             .arg(fontSize)
                             .arg(fontWeight));
}



void MainWindow::onUAVConnected() {
    Logger::instance().log("UAV bağlantı işlemi başlatıldı.");

    // UAV bağlandıktan sonra telemetri sinyalini güncelleme fonksiyonuna bağla
    auto& telemetryHandler = uavManager->getTelemetryHandler();

    if (!telemetryHandler) {
        Logger::instance().log("HATA: TelemetryHandler bulunamadı!", ERROR);
        qDebug() << "TelemetryHandler bulunamadı!";
        return;
    }

    bool connected = connect(telemetryHandler.get(), &TelemetryHandler::telemetryDataUpdated, this, &MainWindow::updateTelemetryData);

    if (!connected) {
        Logger::instance().log("HATA: Telemetri sinyali bağlantısı başarısız!", ERROR);
        qDebug() << "Sinyal bağlantısı başarısız!";
    } else {
        Logger::instance().log("Telemetri sinyali bağlantısı başarılı.");
        qDebug() << "Sinyal bağlantısı başarılı!";
    }

    Logger::instance().log("TelemetryHandler sinyali updateTelemetryData fonksiyonuna bağlandı.");
    qDebug() << "TelemetryHandler sinyali updateTelemetryData fonksiyonuna bağlandı.";
}


void MainWindow::cameraConnectPushButton_clicked()
{
    if (cameraManager->isCameraConnected()) {
        Logger::instance().log("Kamera bağlantısı kesiliyor.");
        cameraManager->disconnectCamera();
    } else {
        QString cameraName = ui->cameraComboBox->currentText();
        Logger::instance().log("Kamera bağlanıyor: " + cameraName);
        cameraManager->connectToCamera(cameraName);
        captureSession = cameraManager->getCaptureSession();
        captureSession->setVideoOutput(videoWidget);
    }
}

void MainWindow::showTime(){
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("hh:mm:ss");
    ui->timeLabel->setText(formattedDateTime);
}

void MainWindow::updateCoordinates(double lat, double lon)
{
    togoLat = lat;
    togoLon = lon;
    Logger::instance().log("Yeni hedef koordinatlar: " + QString::number(lat) + ", " + QString::number(lon));
    ui->coordinateLabel->setText(QString("%1  %2").arg(lat).arg(lon));
}


void MainWindow::updateUAVPosition(double latitude, double longitude, double headingDegrees)
{
    Logger::instance().log("UAV pozisyon güncellemesi: " + QString::number(latitude) + ", " + QString::number(longitude) + " Heading: " + QString::number(headingDegrees));
    QObject *rootObject = ui->quickWidget->rootObject();
    QMetaObject::invokeMethod(rootObject, "updateUAVCoordinate",
                              Q_ARG(QVariant, latitude),
                              Q_ARG(QVariant, longitude),
                              Q_ARG(QVariant, headingDegrees));
}




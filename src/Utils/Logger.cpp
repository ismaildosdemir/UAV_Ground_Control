#include "Logger.h"

QMutex Logger::mutex;  // Statik mutex tanımlaması


Logger::Logger()
    : logDirectory(QDir::currentPath()) // Varsayılan olarak geçerli dizin
{
    // Log dosyasının başlangıç adı
    logFileName = "application.log";
    rotateLogFile();
}

Logger::~Logger()
{
    // Log dosyasını kapatıyoruz.
    if (logFile.isOpen()) {
        logFile.close();
    }
}

Logger& Logger::instance()
{
    static Logger instance;  // Logger sınıfının tek örneğini oluşturuyoruz
    return instance;
}



std::pair<QString, LogLevel> Logger::getLastLog() const {
    return {lastStatusMessage, lastStatusLevel};
}




void Logger::log(const QString &message, LogLevel level)
{




    lastStatusMessage = message;  // Son durum mesajını saklamak için
    lastStatusLevel = level; // Son durum seviyesini saklamak için
    emit StatusDataUpdated();


    QMutexLocker locker(&mutex);  // Log yazarken eş zamanlı erişimi engellemek için

    if (!logFile.isOpen()) {
        rotateLogFile(); // Eğer dosya kapalıysa yeni bir dosya açıyoruz
    }

    if (logFile.isOpen()) {
        QTextStream out(&logFile);
        QString levelStr;
        switch (level) {
        case INFO: levelStr = "INFO"; break;
        case DEBUG: levelStr = "DEBUG"; break;
        case ERROR: levelStr = "ERROR"; break;
        case WARNING: levelStr = "WARNING"; break;
            break;
        }

        out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " - "
            << levelStr << " - " << message << "\n";
    }
}

// MAVSDK log seviyesini destekleyen overload fonksiyon
void Logger::log(const QString &message, mavsdk::log::Level mavsdkLevel)
{
    log(message, mavsdkLogLevelToLogger(mavsdkLevel)); // Dönüştürüp ana log fonksiyonunu çağır
}



void Logger::setLogFilePath(const QString &path)
{
    QMutexLocker locker(&mutex);  // Thread-safe hale getirmek için
    logDirectory = path;
    rotateLogFile(); // Yolu değiştirdikten sonra log dosyasını yeniden açıyoruz.
}

void Logger::rotateLogFile()
{
    QMutexLocker locker(&mutex);  // Thread-safe hale getirmek için

    if (logFile.isOpen()) {
        logFile.close();
    }

    QFileInfo checkFile(QDir(logDirectory).filePath(logFileName));

    // Eğer dosya çok büyükse veya mevcut değilse, yeni bir dosya ismi oluşturulur.
    if (checkFile.exists() && checkFile.size() > 10 * 1024 * 1024) {  // 10MB'tan büyükse
        logFileName = "application_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".log";
    }

    // Yeni log dosyasını açıyoruz
    logFile.setFileName(QDir(logDirectory).filePath(logFileName));
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Log dosyası açılamadı!";
    }
}


void Logger::appendLogMessage(const QString &message , QPlainTextEdit *logTextEdit, const LogLevel &logLevel ) {
    // Get current date and time

    QString logColor;



    switch (logLevel) {
    case LogLevel::DEBUG:
        logColor = "#ff7f50";  // Coral for Debug level
        break;
    case LogLevel::INFO:
        logColor = "#7bed9f";  // Emerald for Info level
        break;
    case LogLevel::WARNING:
        logColor = "#1e90ff";  // Dodger Blue for Warn level
        break;
    case LogLevel::ERROR:
        logColor = "#ff4757";  // Red for Error level
        break;
    default:
        logColor = "black";  // Default color if unknown log level
    }



    // Format log message
    QString formattedMessage = QString("<font color=\"%1\">[%2]</font>")
                                   .arg(logColor, message);

    // Append to the log text edit
    logTextEdit->appendHtml(formattedMessage);
}

void Logger::appendLogMessage(const QString &message , QPlainTextEdit *logTextEdit, mavsdk::log::Level level) {

    appendLogMessage(message , logTextEdit, mavsdkLogLevelToLogger(level));

}



// MAVSDK log seviyesini Logger'ın kendi LogLevel seviyesine dönüştüren yardımcı fonksiyon
LogLevel Logger::mavsdkLogLevelToLogger(mavsdk::log::Level mavsdkLevel)
{
    switch (mavsdkLevel) {
    case mavsdk::log::Level::Debug: return DEBUG;
    case mavsdk::log::Level::Info: return INFO;
    case mavsdk::log::Level::Warn: return WARNING;
    case mavsdk::log::Level::Err: return ERROR;
    default: return INFO; // Varsayılan olarak INFO döndür
    }
}

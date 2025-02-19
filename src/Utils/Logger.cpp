#include "Logger.h"
#include "qplaintextedit.h"
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <mavsdk/log_callback.h>

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

void Logger::log(const QString &message, LogLevel level)
{
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
        }

        out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " - "
            << levelStr << " - " << message << "\n";
    }
}

void Logger::setLogFilePath(const QString &path)
{
    logDirectory = path;
    rotateLogFile(); // Yolu değiştirdikten sonra log dosyasını yeniden açıyoruz.
}

void Logger::rotateLogFile()
{
    // Log dosyasının mevcut dosyayı aşmadığından emin olunuyor.
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

void Logger::appendLogMessage(const QString &message , QPlainTextEdit *logTextEdit, const QString &logLevel , mavsdk::log::Level level) {
    // Get current date and time

    QString logColor;

    if (!logLevel.isEmpty()) {
        // Determine log level color
        if (logLevel == "Debug" && level == mavsdk::log::Level::Debug) {
            logColor = "#ff7f50";
        } else if (logLevel == "Info" && level == mavsdk::log::Level::Info) {
            logColor = "#7bed9f";
        } else if (logLevel == "Warn" && level == mavsdk::log::Level::Warn) {
            logColor = "#1e90ff";
        } else if (logLevel == "Err" && level == mavsdk::log::Level::Err ) {
            logColor = "#ff4757";
        } else {
            logColor = "black"; // Default color
        }
    }else {
        // If logLevel is not provided, check the mavsdk::log::Level (enum) and assign color
        if (level == mavsdk::log::Level::Debug) {
            logColor = "#ff7f50";  // Coral for Debug level
        } else if (level == mavsdk::log::Level::Info) {
            logColor = "#7bed9f";  // Emerald for Info level
        } else if (level == mavsdk::log::Level::Warn) {
            logColor = "#1e90ff";  // Dodger Blue for Warn level
        } else if (level == mavsdk::log::Level::Err) {
            logColor = "#ff4757";  // Red for Error level
        } else {
            logColor = "black";  // Default color if unknown log level
        }
    }

    // Format log message
    QString formattedMessage = QString("<font color=\"%1\">[%2]</font>")
                                   .arg(logColor, message);

    // Append to the log text edit
    logTextEdit->appendHtml(formattedMessage);
}


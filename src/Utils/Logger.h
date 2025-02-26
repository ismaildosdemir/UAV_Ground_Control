#ifndef LOGGER_H
#define LOGGER_H

#include "qplaintextedit.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QDir>
#include <QFileInfo>
#include <mavsdk/log_callback.h>

// Log seviyelerini tanımlıyoruz
enum LogLevel {
    INFO,
    DEBUG,
    ERROR,
    WARNING
};

class Logger
{
public:
    // Logger sınıfının tek örneğini almak için kullanılan fonksiyon
    static Logger& instance();

    // Log mesajlarını dosyaya yazmak için kullanılan fonksiyon
    void log(const QString &message, LogLevel level = INFO);
    void log(const QString &message, mavsdk::log::Level mavsdkLevel); // MAVSDK overload

    // Log dosyasının yolunu değiştirmek için bir fonksiyon
    void setLogFilePath(const QString &path);

    // MAVSDK log seviyelerini Logger seviyelerine dönüştüren yardımcı fonksiyon
    static LogLevel mavsdkLogLevelToLogger(mavsdk::log::Level mavsdkLevel);

    void appendLogMessage(const QString &message,
                          QPlainTextEdit *logTextEdit,
                          const QString &logLevel);

    void appendLogMessage(const QString &message,
                          QPlainTextEdit *logTextEdit,
                          mavsdk::log::Level level);


private:
    Logger(); // Constructor'u private yaparak tekil (singleton) olmasını sağlarız.
    ~Logger();

    // Günlük dosyasının adı
    QString logFileName;
    QString logDirectory;

    // Dosya nesnesi
    QFile logFile;

    // Mutex nesnesi - paralel yazma işlemleri için
    static QMutex mutex;

    // Log dosyasının rotasyonu için kullanılan fonksiyon
    void rotateLogFile();
    QString levelToString(mavsdk::log::Level level);


};
#endif // LOGGER_H





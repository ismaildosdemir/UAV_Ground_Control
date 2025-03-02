#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QPlainTextEdit>
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

class Logger : public QObject // QObject'ten türetildi
{
    Q_OBJECT  // Sinyal-slot kullanabilmek için bu makro gerekli

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
                          const LogLevel &logLevel);

    void appendLogMessage(const QString &message,
                          QPlainTextEdit *logTextEdit,
                          mavsdk::log::Level level);

    QString lastStatusMessage;  // Son durum mesajını saklamak için
    LogLevel lastStatusLevel; // Son durum seviyesini saklamak için
    std::pair<QString, LogLevel> getLastLog() const;

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

signals:
    void StatusDataUpdated();  // Sinyal tanımlandı

};
#endif // LOGGER_H

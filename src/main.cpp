#include <QApplication>
#include "MainWindow/MainWindow.h"
#include "Utils/Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Logger başlatılıyor ve uygulama başlatıldığını belirten mesaj yazılıyor
    Logger::instance().log("Uygulama başlatılıyor...");

    // Ana pencereyi oluştur ve göster
    MainWindow mainWindow;
    mainWindow.show();

    // Uygulama başlatma mesajı
    Logger::instance().log("Ana pencere gösterildi, uygulama çalışıyor.");

    // Uygulamayı çalıştır
    int result = app.exec();

    // Uygulama sona eriyor mesajı
    Logger::instance().log("Uygulama kapatılıyor...");

    return result;
}

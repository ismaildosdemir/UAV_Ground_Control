QT += core gui widgets network multimedia multimediawidgets
QT += location quickwidgets
QT += serialport svg svgwidgets
QT += webenginewidgets webchannel
QT += multimedia-private



greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



# Kaynak dosyaları
SOURCES += \
    src/Camera/CameraManager.cpp \
    src/main.cpp \
    src/MainWindow/MainWindow.cpp \
    src/UAV/UAVManager.cpp \
    src/Telemetry/TelemetryHandler.cpp \
    src/Utils/Logger.cpp \

# Header dosyaları
HEADERS += \
    src/Camera/CameraManager.h \
    src/MainWindow/MainWindow.h \
    src/UAV/UAVManager.h \
    src/Telemetry/TelemetryHandler.h \
    src/Utils/Logger.h

# UI dosyaları
FORMS += \
    src/MainWindow/MainWindow.ui

# Kaynak ve stil dosyaları (örnek: resim ve CSS dosyaları)
RESOURCES += \
    resources/resources.qrc

# Kütüphane yolları ve ekleme (MAVSDK veya diğer üçüncü taraf kütüphaneler)
# Include the MAVSDK headers
INCLUDEPATH += /home/efe/MAVSDK/install/include
INCLUDEPATH += /home/efe/MAVSDK/install/include/mavsdk

# Add the MAVSDK library path
LIBS += -L/home/efe/MAVSDK/install/lib

# Link against the MAVSDK library
LIBS += -lmavsdk

CONFIG += lrelease
CONFIG += embed_translations


include(src/qfi/qfi.pri)


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    README.md

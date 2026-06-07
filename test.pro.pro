QT       += core gui charts sql printsupport network multimedia multimediawidgets serialport texttospeech quick quickwidgets location positioning

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Ne pas traiter les avertissements comme des erreurs (évite Error 1 sur certains warnings)
QMAKE_CXXFLAGS += -Wno-error

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    quai_arduino_window.cpp \
    arduino.cpp \
    arduino_montage_2.cpp \
    arduinouno.cpp \
    connection.cpp \
    employee.cpp \
    pecheur.cpp \
    stock.cpp \
    bateau.cpp \
    bateau_module.cpp \
    esp32_gps.cpp \
    rfid_pecheur.cpp \
    bateaudb.cpp \
    equipement.cpp \
    quai.cpp \
    stock_balance.cpp \
    core_stock.cpp \
    equip_http_server.cpp

HEADERS += \
    mainwindow.h \
    quai_arduino_window.h \
    arduino.h \
    arduino_montage_2.h \
    arduinouno.h \
    connection.h \
    employee.h \
    pecheur.h \
    stock.h \
    bateau.h \
    bateau_module.h \
    esp32_gps.h \
    rfid_pecheur.h \
    bateaudb.h \
    equipement.h \
    equip_http_server.h \
    quai.h \ 
    libraries.h

FORMS += \
    mainwindow.ui \
    quai_arduino_window.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc
#-------------------------------------------------
#
# Project created by QtCreator 2018-08-13T18:49:17
#
#-------------------------------------------------

QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
QT += serialport
QT += autoupdatergui

TARGET = "IQ Control Center"
TEMPLATE = app

QMAKE_CFLAGS = -Wno-unused-parameter
QMAKE_CXXFLAGS = -Wno-unused-parameter

#RC_ICONS = icons/IQ.ico
ICON = icons/IQ.icns
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += MAJOR=1 \
           MINOR=0 \
           PATCH=0

#DEFINES += MAINTENANCETOOL_PATH=\\\"../maintenancetool.exe\\\"
DEFINES += MAINTENANCETOOL_PATH=\\\"../../../maintenancetool.app\\\"
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
#CONFIG += static

#CONFIG-=app_bundle
#QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    switch.cpp \
    IQ_api/client.cpp \
    IQ_api/client_helpers.cpp \
    IQ_api/generic_interface.cpp \
    IQ_api/json.cpp \
    IQ_api/json_cpp.cpp \
    IQ_api/byte_queue.c \
    IQ_api/crc_helper.c \
    IQ_api/packet_finder.c \
    frame_combo.cpp \
    qserial_interface.cpp \
    tab.cpp \
    frame_variables.cpp \
    frame_spin_box.cpp \
    frame.cpp \
    frame_switch.cpp \
    port_connection.cpp \
    defaults.cpp \
    firmware.cpp \
    frame_testing.cpp \
    custom_double_spinbox.cpp \
    custom_combo_box.cpp \
    updater.cpp \
    frame_button.cpp \
    tab_populator.cpp

HEADERS += \
        mainwindow.h \
    switch.h \
    IQ_api/bipbuffer.h \
    IQ_api/byte_queue.h \
    IQ_api/client.hpp \
    IQ_api/client_communication.hpp \
    IQ_api/client_helpers.hpp \
    IQ_api/communication_interface.h \
    IQ_api/crc_helper.h \
    IQ_api/generic_interface.hpp \
    IQ_api/json-forwards.h \
    IQ_api/json.h \
    IQ_api/json_cpp.hpp \
    IQ_api/packet_finder.h \
    frame_combo.h \
    qserial_interface.hpp \
    tab.h \
    frame_variables.h \
    frame_spin_box.h \
    frame.h \
    frame_switch.h \
    main.h \
    port_connection.hpp \
    defaults.h \
    firmware.h \
    frame_testing.h \
    custom_double_spinbox.h \
    custom_combo_box.h \
    updater.hpp \
    frame_button.hpp \
    tab_populator.hpp

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc \
    clients.qrc

#LIBS += -framework CoreFoundation



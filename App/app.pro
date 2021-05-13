#-------------------------------------------------
#
# Project created by QtCreator 2018-08-13T18:49:17
#
#-------------------------------------------------

QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport
QT += serialport
#QT += autoupdatergui

TARGET = "IQ Control Center"
TEMPLATE = app

QMAKE_CFLAGS = -Wno-unused-parameter
QMAKE_CXXFLAGS = -Wno-unused-parameter

# for windows
win32{
RC_ICONS = icons/IQ.ico
}
# for mac
macx {
ICON = icons/IQ.icns
}

unix {
ICON = icons/IQ.icns
}
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += MAJOR=1 \
           MINOR=2 \
           PATCH=0

win32 {
DEFINES += MAINTENANCETOOL_PATH=\\\"../maintenancetool.exe\\\"
}
macx {
DEFINES += MAINTENANCETOOL_PATH=\\\"../../../maintenancetool.app\\\"
}

unix {
DEFINES += MAINTENANCETOOL_PATH=\\\"../maintenancetool\\\"
}
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG += static


SOURCES += \
    Schmi/binary_file_std.cpp \
    Schmi/flash_loader.cpp \
    Schmi/loading_bar_std.cpp \
    Schmi/qerror_handler.cpp \
    Schmi/qserial.cpp \
    Schmi/stm32.cpp \
    flash_loading_bar.cpp \
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
    frame_button.cpp \
    tab_populator.cpp \


HEADERS += \
    Schmi/binary_file_interface.hpp \
    Schmi/binary_file_std.hpp \
    Schmi/error_handler_interface.hpp \
    Schmi/flash_loader.hpp \
    Schmi/loading_bar_interface.hpp \
    Schmi/loading_bar_std.hpp \
    Schmi/qerror_handler.hpp \
    Schmi/qserial.h \
    Schmi/serial_interface.hpp \
    Schmi/std_exception.hpp \
    Schmi/stm32.hpp \
    flash_loading_bar.h \
    frame_button.h \
        mainwindow.h \
    port_connection.h \
    qserial_interface.h \
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
    tab.h \
    frame_variables.h \
    frame_spin_box.h \
    frame.h \
    frame_switch.h \
    main.h \
    defaults.h \
    firmware.h \
    frame_testing.h \
    custom_double_spinbox.h \
    custom_combo_box.h \
    tab_populator.h \

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



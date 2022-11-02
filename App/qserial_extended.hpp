#ifndef QSERIALEXTENDED_HPP
#define QSERIALEXTENDED_HPP

#include "qserial_interface.h"
#include "schmi/include/Schmi/std_exception.hpp"
#include "schmi/include/Schmi/qserial.h"

namespace SerialConst {
const qint32 BAUD_RATE = QSerialPort::Baud115200;
};

struct SerialReadData {
  uint8_t* buffer;
  uint16_t bytes_left;
};

//This class is going to live below QSerialInterface, and will add core read and write operations to the serial line
class QSerialExtended : public QSerialInterface, public Schmi::QSerial{

public:
    QSerialExtended(const QString& usb_handle, const qint32 &baud_rate = 115200):
        QSerialInterface(usb_handle, baud_rate),  //Let the top class make and open the serial port
        Schmi::QSerial(ser_port_),
        qser_port_(ser_port_),
        usb_handle_(usb_handle)
    {
    };

    ~QSerialExtended();

    QSerialPort* qser_port_;

private:
    QString usb_handle_;

};

#endif // QSERIALEXTENDED_HPP

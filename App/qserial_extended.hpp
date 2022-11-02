#ifndef QSERIALEXTENDED_HPP
#define QSERIALEXTENDED_HPP

#include "qserial_interface.h"
#include "schmi/include/Schmi/std_exception.hpp"

namespace SerialConst {
const qint32 BAUD_RATE = QSerialPort::Baud115200;
};

struct SerialReadData {
  uint8_t* buffer;
  uint16_t bytes_left;
};

//This class is going to live below QSerialInterface, and will add core read and write operations to the serial line
class QSerialExtended : public QSerialInterface{

public:
    QSerialExtended(const QString& usb_handle, const qint32 &baud_rate = 115200):
        QSerialInterface(usb_handle, baud_rate),  //Let the top class make and open the serial port
        qser_port_(ser_port_),
        usb_handle_(usb_handle)
    {
    };

    ~QSerialExtended();

    int Write(uint8_t* buffer, const uint16_t& buffer_length);
    int Read(uint8_t* buffer, const uint16_t& num_bytes, const uint16_t& timeout_ms = 500);
    void Init();

    QSerialPort* qser_port_;

private:
    QString usb_handle_;

    void CheckNumBytesWritten(const qint64& num_bytes_written, const uint16_t& buffer_length);
    void CheckNumBytesRead(const qint64& num_bytes_read);
    void UpdateSerialReadData(SerialReadData& read_data, const qint64& num_bytes_read);
};

#endif // QSERIALEXTENDED_HPP

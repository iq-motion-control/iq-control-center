
#include "qserial_extended.hpp"
#include "main.h"

////Public Functions
//int QSerialExtended::Write(uint8_t* buffer, const uint16_t& buffer_length){
//    try {
//      QByteArray byte_array = QByteArray(reinterpret_cast<char*>(buffer), (int)(buffer_length));
//      qint64 num_bytes_written = qser_port_->write(byte_array);
//      qser_port_->waitForBytesWritten(-1);

//      CheckNumBytesWritten(num_bytes_written, buffer_length);

//    } catch (const Schmi::StdException& e) {
//      iv.label_message->setText(e.what());
//      return -1;
//    }
//    return 0;
//  }

//int QSerialExtended::Read(uint8_t* buffer, const uint16_t& num_bytes, const uint16_t& timeout_ms) {
//  SerialReadData read_data = {buffer, num_bytes};
//  try {
//    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
//    while (read_data.bytes_left) {
//      if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(timeout_ms)) {
//        std::stringstream err_message;
//        err_message << "Read Timout: " << timeout_ms;
//        throw Schmi::StdException(err_message.str());
//      }

//      qser_port_->waitForReadyRead(10);
//      qint64 num_bytes_read =
//          qser_port_->read(reinterpret_cast<char*>(read_data.buffer), read_data.bytes_left);

//      CheckNumBytesRead(num_bytes_read);
//      UpdateSerialReadData(read_data, num_bytes_read);
//    }

//  } catch (const Schmi::StdException& e) {
//    iv.label_message->setText(e.what());
//    return -1;
//  }
//  return 0;
//}

////Private Functions
//void QSerialExtended::CheckNumBytesWritten(const qint64& num_bytes_written, const uint16_t& buffer_length) {
//    if (num_bytes_written != buffer_length) {
//      std::stringstream err_message;
//      err_message << "Error writting bytes: " << num_bytes_written;
//      err_message << " / wanted to write: " << (int)buffer_length;
//      throw Schmi::StdException(err_message.str());
//    }
//}

//void QSerialExtended::CheckNumBytesRead(const qint64& num_bytes_read) {
//    if (num_bytes_read < 0) {
//      std::stringstream err_message;
//      err_message << "Error reading bytes: " << num_bytes_read;
//      throw Schmi::StdException(err_message.str());
//    }

//    return;
//}

//void QSerialExtended::UpdateSerialReadData(SerialReadData& read_data, const qint64& num_bytes_read) {
//    read_data.bytes_left -= num_bytes_read;
//    read_data.buffer += num_bytes_read;

//    return;
//}

//Deconstructor
QSerialExtended::~QSerialExtended(){
    if(qser_port_->isOpen()){
        qser_port_->close();
    }

    delete qser_port_;
}


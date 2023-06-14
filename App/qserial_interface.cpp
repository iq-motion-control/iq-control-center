/*
    Copyright 2018 - 2019 IQ Motion Control   	dskart11@gmail.com

    This file is part of IQ Control Center.

    IQ Control Center is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    IQ Control Center is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "qserial_interface.h"

#include <iostream>

QSerialInterface::QSerialInterface(){}

QSerialInterface::QSerialInterface(const QString& dev, const qint32& baud_rate) {
    InitSerial(dev, baud_rate);
}

void QSerialInterface::InitSerial(const QString &dev, const qint32 &baud_rate){
    InitBQ(&index_queue, pf_index_data, SERIAL_PF_INDEX_DATA_SIZE);
    InitPacketFinder(&pf, &index_queue);
    tx_bipbuf = BipBuffer(tx_buffer, SERIAL_TX_BUFFER_SIZE);

    ser_port_ = new QSerialPort(dev);
    ser_port_->setBaudRate(baud_rate, QSerialPort::AllDirections);
    ser_port_->setDataBits(QSerialPort::Data8);
    ser_port_->setParity(QSerialPort::NoParity);
    ser_port_->setStopBits(QSerialPort::OneStop);
    LinkSerialPort(ser_port_);
}

int8_t QSerialInterface::GetBytes() {
  ser_port_->waitForReadyRead(0);
  qint16 num_bytes_read_queue = ser_port_->bytesAvailable();

  while (num_bytes_read_queue) {
    QByteArray read_buffer_qarray =
        ser_port_->read(std::min((int)num_bytes_read_queue, SERIAL_RX_BUFFER_SIZE));

    uint8_t read_buffer[SERIAL_RX_BUFFER_SIZE];
    for (int i = 0; i < read_buffer_qarray.size(); ++i) {
      read_buffer[i] = (uint8_t)(read_buffer_qarray.at(i));
    }

    QSerialInterface::SetRxBytes(read_buffer, (int)read_buffer_qarray.size());

    num_bytes_read_queue = ser_port_->bytesAvailable();
  }
  return 0;
}

int8_t QSerialInterface::SetRxBytes(uint8_t* data_in, uint16_t length_in) {
  if (data_in == nullptr) return -1;

  if (length_in) {
    // copy it over
    PutBytes(&pf, data_in, length_in);
    return 1;
  } else
    return 0;
}

int8_t QSerialInterface::PeekPacket(uint8_t** packet, uint8_t* length) {
  return (::PeekPacket(&pf, packet, length));
}

int8_t QSerialInterface::DropPacket() { return (::DropPacket(&pf)); }

int8_t QSerialInterface::SendPacket(uint8_t msg_type, uint8_t* data, uint16_t length) {
  // This function must not be interrupted by another call to SendBytes or
  // SendPacket, or else the packet it builds will be spliced/corrupted.

  uint8_t header[3];
  header[0] = kStartByte;  // const defined by packet_finder.c
  header[1] = length;
  header[2] = msg_type;
  SendBytes(header, 3);

  SendBytes(data, length);

  uint8_t footer[2];
  uint16_t crc;
  crc = MakeCrc(&(header[1]), 2);
  crc = ArrayUpdateCrc(crc, data, length);
  footer[0] = crc & 0x00FF;
  footer[1] = crc >> 8;
  SendBytes(footer, 2);

  return (1);
}

int8_t QSerialInterface::SendBytes(uint8_t* bytes, uint16_t length) {
  uint16_t length_temp = 0;
  uint8_t* location_temp;
  int8_t ret = 0;

  // Reserve space in the buffer
  location_temp = tx_bipbuf.Reserve(length, length_temp);

  // If there's room, do the copy
  if (length == length_temp) {
    memcpy(location_temp, bytes, length_temp);  // do copy
    tx_bipbuf.Commit(length_temp);
    ret = 1;
  } else {
    tx_bipbuf.Commit(0);  // Call the restaurant, cancel the reservations
  }

  return ret;
}

int8_t QSerialInterface::GetTxBytes(uint8_t* data_out, uint8_t& length_out) {
  uint16_t length_temp;
  uint8_t* location_temp;

  location_temp = tx_bipbuf.GetContiguousBlock(length_temp);
  if (length_temp) {
    memcpy(data_out, location_temp, length_temp);
    length_out = length_temp;
    tx_bipbuf.DecommitBlock(length_temp);

    location_temp = tx_bipbuf.GetContiguousBlock(length_temp);
    memcpy(&data_out[length_out], location_temp, length_temp);
    length_out = length_out + length_temp;
    tx_bipbuf.DecommitBlock(length_temp);
    return 1;
  }
  return 0;
}

void QSerialInterface::SendNow() {
  // local variables
  uint8_t write_communication_buffer[256];
  uint8_t write_communication_length;

  QSerialInterface::GetTxBytes(write_communication_buffer, write_communication_length);

  QByteArray ba = QByteArray(reinterpret_cast<char*>(write_communication_buffer),
                             (int)(write_communication_length));

  ser_port_->write(ba);
  ser_port_->waitForBytesWritten(-1);
}

void QSerialInterface::ReadMsg(CommunicationInterface& com, uint8_t* data, uint8_t length) {
  // I currently don't support being talked to
}

int QSerialInterface::GetRawBytes() {
  // I'm not supported yet
  return 0;
}

void QSerialInterface::Flush() {
  uint8_t* rx_data;       // temporary pointer to received type+data bytes
  uint8_t rx_length = 1;  // number of received type+data bytes

  while (rx_length > 0) {
    GetBytes();
    rx_length = 0;
    while (PeekPacket(&rx_data, &rx_length)) {
      DropPacket();
    }
  }
}

QSerialInterface::~QSerialInterface() {
  if (ser_port_->isOpen()) {
    ser_port_->close();
  }
  delete ser_port_;
}

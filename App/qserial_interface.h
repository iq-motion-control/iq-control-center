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

#ifndef QSERIAL_INTERFACE_H
#define QSERIAL_INTERFACE_H

#include <algorithm>
#include <iterator>

#include "IQ_api/communication_interface.h"
#include "IQ_api/packet_finder.h"
#include "IQ_api/byte_queue.h"
#include "IQ_api/bipbuffer.h"
#include "IQ_api/crc_helper.h"
#include "string.h" // for memcpy

#include "iq_flasher/include/Schmi/qserial.h"

#include "QString"
#include "QSerialPort"

#define SERIAL_PF_INDEX_DATA_SIZE 20   // size of index buffer in packet_finder

#ifndef SERIAL_TX_BUFFER_SIZE
  #define SERIAL_TX_BUFFER_SIZE 65535 //256
#endif

#ifndef SERIAL_RX_BUFFER_SIZE
  #define SERIAL_RX_BUFFER_SIZE 65535 //256
#endif

class QSerialInterface: public CommunicationInterface, public Schmi::QSerial/*, public QObject*/
{
  public:
    QSerialPort *ser_port_;
    // Member Variables
    struct PacketFinder pf;        // packet_finder instance
    struct ByteQueue index_queue;              // needed by pf for storing indices
    uint8_t pf_index_data[SERIAL_PF_INDEX_DATA_SIZE]; // data for index_queue used by pf
    BipBuffer tx_bipbuf;   // bipbuffer for transmissions
    uint8_t tx_buffer[SERIAL_TX_BUFFER_SIZE];   // raw buffer for transmissions

    // READING FROM HARDWARE
    int8_t GetBytes();

    int GetRawBytes();

    // Default Constructor
    QSerialInterface();
    QSerialInterface(const QString &dev, const qint32 &baud_rate = 115200);
    ~QSerialInterface();

    void InitSerial(const QString &dev, const qint32 &baud_rate = 115200);

    /*******************************************************************************
     * Receive
     ******************************************************************************/

    /// Poll the hardware for new byte data.
    ///   Returns: 1 packet ready
    ///            0 normal operation
    ///           -1 failure
    ///

    int8_t SetRxBytes(uint8_t* data_in, uint16_t length_in);

    /// Peek at the next available incoming packet. If a packet is ready, pointer
    /// 'packet' will point to the first byte of type+data and 'length' will give
    /// the length of packet type+data. Arguments 'packet' and 'length' are ignored
    /// if no packet is ready.  Repeated calls to Peek will return pointers to the
    /// same packet data until Drop is used.
    ///   Returns: 1 packet peek available
    ///            0 no packet peek available
    ///           -1 failure
    ///
    int8_t PeekPacket(uint8_t **packet, uint8_t *length);

    /// Drop the next available packet from queue. Usually called after Peek.
    ///   Returns: 1 packet removed
    ///            0 no packet ready to remove
    ///           -1 failure
    ///
    int8_t DropPacket();

    /*******************************************************************************
     * Send
     ******************************************************************************/

    /// Add a packet to the outgoing queue with automatically generated header
    /// and CRC. If the buffer fills, the most recent data is lost.
    int8_t SendPacket(uint8_t msg_type, uint8_t *data, uint16_t length);

    /// Add bytes to the outgoing queue.
    /// If the buffer fills, the most recent data is lost.
    int8_t SendBytes(uint8_t *bytes, uint16_t length);

    /// Send data through hardware
    void SendNow();

    /// Gets all outbound bytes
    /// The data is copied into the user supplied data_out buffer.
    /// The length of data transferred is copied into length_out.
    /// Returns: 1 for data transferred
    ///          0 for no data transferred (buffer empty)
    int8_t GetTxBytes(uint8_t* data_out, uint8_t& length_out);

    /*******************************************************************************
     * Parsing
     ******************************************************************************/

    /// Read a given message and act appropriately.
    void ReadMsg(CommunicationInterface& com, uint8_t* data, uint8_t length);

    void Flush();
};

#endif // QSERIAL_INTERFACE_H

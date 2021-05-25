#ifndef SCHMI_STM32_HPP
#define SCHMI_STM32_HPP

#include "Schmi/error_handler_interface.hpp"
#include "Schmi/serial_interface.hpp"

#include <math.h> /* floor */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace Schmi {

// All these values are from the AN3155 file
namespace CMD {
const uint8_t USART_INIT[2] = {0x7F, 0x00};  // added dummy 0x00 byte so all commands are 2 bytes long
const uint8_t ACK = 0x79;
const uint8_t NACK = 0x1F;
const uint8_t GET[2] = {0x01, 0xFE};
const uint8_t GET_VER_PROTECT_STATUS[2] = {0x01, 0xFE};
const uint8_t GET_ID[2] = {0x02, 0xFD};
const uint8_t READ_MEMORY[2] = {0x11, 0xEE};
const uint8_t GO[2] = {0x21, 0xDE};
const uint8_t WRITE_MEMORY[2] = {0x31, 0xCE};
const uint8_t ERASE[2] = {0x43, 0xBC};
const uint8_t EXTEND_ERASE[2] = {0x44, 0xBB};
const uint8_t WRITE_PROTECT[2] = {0x63, 0x9C};
const uint8_t WRITE_UNPROTECT[2] = {0x73, 0x8C};
const uint8_t READOUT_PROTECT[2] = {0x82, 0x7D};
const uint8_t READOUT_UNPROTECT[2] = {0x92, 0x6D};
}  // namespace CMD

struct VersionAndReadProtectionData {
  uint8_t version;
  uint8_t option1;
  uint8_t option2;
};

const uint16_t MAX_MESSAGE_SIZE = 512;

class Stm32 {
 public:
  // const uint16_t MAX_MESSAGE_LENGTH = 512;

  Stm32(SerialInterface& ser, ErrorHandlerInterface& error) : ser_(ser), error_handler_(error){};
  ~Stm32(){};

  bool InitUsart();

  // bool Get();

  bool GetVersionAndReadProtection(VersionAndReadProtectionData& vrpd);

  bool GetID(uint16_t& id);

  // The max value of num_bytes_to_read = 256
  bool ReadMemory(uint8_t* bytes_read_buffer, const uint16_t num_bytes_to_read,
                  const uint32_t& address);

  bool GoToAddress(const uint32_t& address);

  bool WriteMemory(uint8_t* bytes, const uint16_t& num_bytes, const uint32_t& start_address);

  // Multiple ExtendedErase commands will be sent if num_of_pages > 254
  bool ExtendedErase(uint16_t* page_codes, const uint16_t& num_of_pages);

  bool SpecialExtendedErase(const uint16_t& special_extended_erase_code);

  // bool  WriteProtect();

  // bool WriteUnprotected();

  bool ReadoutUnprotect();

  // bool ReadoutProcted();

 private:
  SerialInterface& ser_;
  ErrorHandlerInterface& error_handler_;

  // We are blocking it to 512, for extended erase it will send multiple messages in a row
  uint8_t message_buffer[MAX_MESSAGE_SIZE];

  bool SendAddressMessage(const uint32_t& address);
  bool SendWriteMemoryBytesMessage(uint8_t* bytes, const uint16_t& num_bytes);

  bool SendCmd(const uint8_t* cmd);

  bool SendMessage(uint8_t* message, const size_t& message_length,
                   const uint16_t& ack_read_timeout_ms = 500);
  bool SendBytes(uint8_t* buffer, const size_t& buffer_length);
  bool CheckForAck(const uint16_t& ack_read_timeout_ms = 500);

  bool ReadBytes(uint8_t* buffer, const size_t& num_bytes, const uint16_t& timeout_ms = 500);

  void AddCheckSum(uint8_t* message, const size_t& num_bytes);
  uint8_t CalculateCheckSum(uint8_t* buffer, const size_t& num_bytes);
  bool SpecialExtendedEraseCheckSum(const uint16_t& special_extended_erase_code, uint8_t& checksum);

  VersionAndReadProtectionData CreateVersionAndReadProtection(uint8_t* bytes);
};
}  // namespace Schmi

#endif  // SCHMI_STM32_HPP

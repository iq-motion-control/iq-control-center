#include "Schmi/stm32.hpp"

namespace Schmi {

bool Stm32::InitUsart() {
  if (!SendCmd(CMD::USART_INIT)) {
    return 0;
  }

  return 1;
}

bool Stm32::GetVersionAndReadProtection(VersionAndReadProtectionData& vrpd) {
  if (!SendCmd(CMD::GET_VER_PROTECT_STATUS)) {
    return 0;
  }

  const uint8_t num_incoming_bytes = 4;
  uint8_t incoming_bytes[num_incoming_bytes];
  if (!ReadBytes(incoming_bytes, num_incoming_bytes)) {
    return 0;
  }

  vrpd = CreateVersionAndReadProtection(incoming_bytes);

  return 1;
}

bool Stm32::GetID(uint16_t& id) {
  if (!SendCmd(CMD::GET_ID)) {
    return 0;
  }

  const uint8_t num_incoming_bytes = 4;
  uint8_t incoming_bytes[num_incoming_bytes];
  if (!ReadBytes(incoming_bytes, num_incoming_bytes)) {
    return 0;
  }

  id = (incoming_bytes[1] << 8) | incoming_bytes[2];

  return 1;
}

bool Stm32::ReadMemory(uint8_t* bytes_read_buffer, const uint16_t num_bytes_to_read,
                       const uint32_t& address) {
  if (num_bytes_to_read > 256) {
    Schmi::Error err = {"ReadMemory", "num_bytes_to_read > 256", num_bytes_to_read};
    error_handler_.Init(err);
    error_handler_.DisplayAndDie();
    return 0;
  }

  if (!SendCmd(CMD::READ_MEMORY)) {
    return 0;
  }

  if (!SendAddressMessage(address)) {
    return 0;
  }

  const uint8_t message_length = 2;
  uint8_t num_bytes = num_bytes_to_read - 1;
  uint8_t message[message_length];
  message[0] = num_bytes;
  message[1] = ~num_bytes;

  if (!SendMessage(message, message_length)) {
    return 0;
  }

  uint16_t num_incoming_bytes = num_bytes_to_read;
  if (!ReadBytes(bytes_read_buffer, num_incoming_bytes)) {
    return 0;
  }

  return 1;
}

bool Stm32::GoToAddress(const uint32_t& address) {
  if (!SendCmd(Schmi::CMD::GO)) {
    return 0;
  }

  if (!SendAddressMessage(address)) {
    return 0;
  }

  return 1;
}

bool Stm32::WriteMemory(uint8_t* bytes, const uint16_t& num_bytes, const uint32_t& start_address) {
  if (!SendCmd(CMD::WRITE_MEMORY)) {
    return 0;
  }

  if (!SendAddressMessage(start_address)) {
    return 0;
  }

  if (!SendWriteMemoryBytesMessage(bytes, num_bytes)) {
    return 0;
  }

  return 1;
}

bool Stm32::ExtendedErase(uint16_t* page_codes, const uint16_t& num_of_pages) {
  if (!SendCmd(CMD::EXTEND_ERASE)) {
    return 0;
  }

  // MAX_NUM_PAGES = 254 if MAX_MESSAGE_SIZE = 512
  const uint16_t MAX_NUM_PAGES = floor((MAX_MESSAGE_SIZE / 2) - 3);

  int num_pages_left = num_of_pages;
  uint16_t num_pages_ready_to_erase = 0;
  uint16_t offset = 0;
  while (num_pages_left > 0) {
    if (num_pages_left > MAX_NUM_PAGES) {
      num_pages_ready_to_erase = MAX_NUM_PAGES;
    } else {
      num_pages_ready_to_erase = num_pages_left;
    }

    // This is how the Extend Erase message is made, (look up AN3155)
    // it's confusing but not worth making its own function, just look at the pdf
    // 2 bytes for N+1 pages | 2 bytes per page number | checmsum
    const uint8_t num_pages_message_length = 2;
    const uint8_t checksum_message_length = 1;
    const uint16_t pages_code_message_length = 2 * (num_pages_ready_to_erase);
    uint16_t message_length =
        num_pages_message_length + pages_code_message_length + checksum_message_length;

    message_buffer[0] = ((num_pages_ready_to_erase - 1) >> 8);
    message_buffer[1] = (num_pages_ready_to_erase - 1) & 0xff;
    for (int ii = 0; ii < num_pages_ready_to_erase; ii++) {
      message_buffer[ii * 2 + 2] = (page_codes[offset + ii] >> 8);
      message_buffer[ii * 2 + 3] = page_codes[offset + ii] & 0xff;
    }

    AddCheckSum(message_buffer, message_length);

    const uint16_t ack_read_timeout_ms = 1000;
    if (!SendMessage(message_buffer, message_length, ack_read_timeout_ms)) {
      return 0;
    }

    num_pages_left -= num_pages_ready_to_erase;
    offset += num_pages_ready_to_erase;
  }

  return 1;
}

bool Stm32::SpecialExtendedErase(const uint16_t& special_extended_erase_code) {
  if (!SendCmd(CMD::EXTEND_ERASE)) {
    return 0;
  }

  // This is how the Extend Erase message for special codes is made, (look up AN3155)
  const uint8_t message_length = 3;
  uint8_t message[message_length];

  message[0] = (special_extended_erase_code >> 8);
  message[1] = (special_extended_erase_code & 0xff);
  if (!SpecialExtendedEraseCheckSum(special_extended_erase_code, message[2])) {
    return 0;
  }

  if (!SendMessage(message, message_length)) {
    return 0;
  }

  return 1;
}

bool Stm32::ReadoutUnprotect() {
  if (!SendCmd(CMD::READOUT_UNPROTECT)) {
    return 0;
  }

  if (!CheckForAck()) {
    return 0;
  }

  return 1;
}

bool Stm32::SendAddressMessage(const uint32_t& address) {
  // Check AN3155.pdf for message structure
  const uint8_t message_length = 5;
  uint8_t message[message_length];
  message[3] = address & 0xFF;
  message[2] = (address >> 8) & 0xFF;
  message[1] = (address >> 16) & 0xFF;
  message[0] = (address >> 24) & 0xFF;

  AddCheckSum(message, message_length);

  if (!SendMessage(message, message_length)) {
    return 0;
  }

  return 1;
}

bool Stm32::SendWriteMemoryBytesMessage(uint8_t* bytes, const uint16_t& num_bytes) {
  uint16_t message_length = num_bytes + 2;  // plus first byte and checksum

  message_buffer[0] = num_bytes - 1;
  for (uint16_t ii = 0; ii < num_bytes; ii++) {
    message_buffer[ii + 1] = *bytes++;
  }

  // Pad message array with 0xFF to garantee num_bytes is a multiple of 4 (check datasheet)
  if (num_bytes % 4 != 0) {
    uint8_t num_pad_bytes = (4 - num_bytes % 4);
    uint16_t padded_num_bytes = num_bytes + num_pad_bytes;

    if (padded_num_bytes > 256 || padded_num_bytes % 4 != 0) {
      Schmi::Error err = {"SendWriteMemoryBytesMessage", "num_byte > 256 || !%4", padded_num_bytes};
      error_handler_.Init(err);
      error_handler_.DisplayAndDie();
      return 0;
    }

    for (uint16_t ii = num_bytes + 1; ii <= padded_num_bytes; ii++) {
      message_buffer[ii] = 0xFF;
    }

    message_length += num_pad_bytes;
  }

  AddCheckSum(message_buffer, message_length);

  if (!SendMessage(message_buffer, message_length)) {
    return 0;
  }

  return 1;
}

bool Stm32::SendCmd(const uint8_t* cmd) {
  const uint8_t message_length = 2;
  uint8_t message[message_length];
  memcpy(message, cmd, message_length);

  if (!SendMessage(message, message_length)) {
    return 0;
  }

  return 1;
}

bool Stm32::SendMessage(uint8_t* message, const size_t& message_length,
                        const uint16_t& ack_read_timeout_ms) {
  if (!SendBytes(message, message_length)) {
    return 0;
  }
  if (!CheckForAck(ack_read_timeout_ms)) {
    return 0;
  }

  return 1;
}

bool Stm32::SendBytes(uint8_t* buffer, const size_t& buffer_length) {
  if (ser_.Write(buffer, buffer_length) != 0) {
    Schmi::Error err = {"SendBytes", "Failed to send Bytes", -1};
    error_handler_.Init(err);
    //    error_handler_.DisplayAndDie();
    error_handler_.Display();
    return 0;
  }

  return 1;
}

bool Stm32::CheckForAck(const uint16_t& ack_read_timeout_ms) {
  const uint8_t num_bytes_to_read = 1;
  uint8_t buffer[num_bytes_to_read];
  if (!ReadBytes(buffer, num_bytes_to_read, ack_read_timeout_ms)) {
    return 0;
  }

  if (*buffer != CMD::ACK) {
    Schmi::Error err = {"CheckForAck", "Not ACK", *buffer};
    error_handler_.Init(err);
    error_handler_.DisplayAndDie();
    return 0;
  }

  return 1;
}

bool Stm32::ReadBytes(uint8_t* buffer, const size_t& num_bytes, const uint16_t& timeout_ms) {
  int result = ser_.Read(buffer, num_bytes, timeout_ms);
  if (result != 0) {
    Schmi::Error err = {"ReadBytes", "Failed to read Bytes", result};
    error_handler_.Init(err);
    //    error_handler_.DisplayAndDie();
    error_handler_.Display();
    return 0;
  }

  return 1;
}

void Stm32::AddCheckSum(uint8_t* message, const size_t& message_length) {
  uint8_t checksum = CalculateCheckSum(message, message_length - 1);
  message[message_length - 1] = checksum;

  return;
}

uint8_t Stm32::CalculateCheckSum(uint8_t* buffer, const size_t& num_bytes) {
  uint8_t checksum = 0;

  for (size_t ii = 0; ii < num_bytes; ii++) {
    checksum = buffer[ii] ^ checksum;
  }

  return checksum;
}

bool Stm32::SpecialExtendedEraseCheckSum(const uint16_t& special_extended_erase_code,
                                         uint8_t& checksum) {
  switch (special_extended_erase_code) {
    case 0xFFFF:  // global errase
      checksum = 0x00;
      break;

    case 0xFFFE:  // bank 1 mass erase
      checksum = 0x01;
      break;

    case 0xFFFD:  // bank 2 mass erase
      checksum = 0x02;
      break;

    default:
      Schmi::Error err = {"SpecialExtendedErase", "Code not recognized",
                          special_extended_erase_code};
      error_handler_.Init(err);
      error_handler_.DisplayAndDie();
      return 0;
  }

  return 1;
}

VersionAndReadProtectionData Stm32::CreateVersionAndReadProtection(uint8_t* bytes) {
  VersionAndReadProtectionData version_read_protection;
  version_read_protection.version = bytes[0];
  version_read_protection.option1 = bytes[1];
  version_read_protection.option2 = bytes[2];

  return version_read_protection;
}
}  // namespace Schmi

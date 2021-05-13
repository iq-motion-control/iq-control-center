#include "Schmi/flash_loader.hpp"
#include <QDebug>

namespace Schmi {

void FlashLoader::Init() {
  ser_->Init();
  bin_->Init();
  total_num_bytes_ = bin_->GetBinaryFileSize();

  return;
}

bool FlashLoader::InitUsart() { return stm32_->InitUsart(); }

bool FlashLoader::Flash(bool init_usart, bool global_erase) {
  if (init_usart) {
    if (!stm32_->InitUsart()) {
      return 0;
    }
  }

  if (global_erase) {
    if (!stm32_->SpecialExtendedErase(0xFFFF)) {
      return 0;
    }
  } else {
    uint16_t num_of_pages = GetPagesCodesFromBinary();
    if (!stm32_->ExtendedErase(pages_codes_buffer, num_of_pages)) {
      return 0;
    }
  }

  if (!FlashBytes()) {
    return 0;
  }

  if (!CheckMemory()) {
    return 0;
  }

  if (!stm32_->GoToAddress(START_ADDRESS_)) {
    return 0;
  }

  return 1;
}

bool FlashLoader::Flash(uint16_t* page_codes, const uint16_t& num_of_pages, bool init_usart) {
  if (init_usart) {
    if (!stm32_->InitUsart()) {
      return 0;
    }
  }

  if (!stm32_->ExtendedErase(page_codes, num_of_pages)) {
    return 0;
  }

  if (!FlashBytes()) {
    return 0;
  }

  if (!CheckMemory()) {
    return 0;
  }

  if (!stm32_->GoToAddress(START_ADDRESS_)) {
    return 0;
  }

  return 1;
}

uint16_t FlashLoader::GetPagesCodesFromBinary() {
  float binary_file_size = bin_->GetBinaryFileSize();
  uint16_t num_of_pages = ceil(binary_file_size / PAGE_SIZE_);

  if (num_of_pages > MAX_NUM_PAGES_TO_ERASE) {
    Schmi::Error err_message = {"GetPagesCodesFromBinary", ("num pages > 512"), num_of_pages};
    err_->Init(err_message);
    err_->DisplayAndDie();
    return 0;
  }

  for (int ii = 0; ii < num_of_pages; ++ii) {
    pages_codes_buffer[ii] = ii;
  }

  return num_of_pages;
}

bool FlashLoader::FlashBytes() {
  BinaryBytesData flash_data = {0, START_ADDRESS_, total_num_bytes_};

  bar_->StartLoadingBar(total_num_bytes_);

  while (flash_data.bytes_left) {
    uint32_t num_bytes = CheckNumBytesToWrite(flash_data.bytes_left);

    uint8_t binary_buffer[MAX_WRITE_SIZE];
    bin_->GetBytesArray(binary_buffer, {num_bytes, flash_data.current_byte_pos});

    if (!stm32_->WriteMemory(binary_buffer, num_bytes, flash_data.current_memory_address)) {
      return 0;
    }

    UpdateBinaryBytesData(flash_data, num_bytes);

    bar_->UpdateLoadingBar(flash_data.bytes_left);
  }

  bar_->EndLoadingBar();

  return 1;
}

bool FlashLoader::CheckMemory() {
  BinaryBytesData memory_data = {0, START_ADDRESS_, total_num_bytes_};

  bar_->StartCheckingLoadingBar(total_num_bytes_);

  while (memory_data.bytes_left) {
    uint16_t num_bytes = CheckNumBytesToWrite(memory_data.bytes_left);

    uint8_t binary_buffer[MAX_WRITE_SIZE];
    bin_->GetBytesArray(binary_buffer, {num_bytes, memory_data.current_byte_pos});

    uint8_t memory_buffer[MAX_WRITE_SIZE];
    if (!stm32_->ReadMemory(memory_buffer, num_bytes, memory_data.current_memory_address)) {
      return 0;
    }
    if (!CompareBinaryAndMemory(memory_buffer, binary_buffer, num_bytes)) {
      return 0;
    }

    UpdateBinaryBytesData(memory_data, num_bytes);

    bar_->UpdateLoadingBar(memory_data.bytes_left);
  }

  bar_->EndLoadingBar();

  return 1;
}

bool FlashLoader::CompareBinaryAndMemory(uint8_t* memory_buffer, uint8_t* binary_buffer,
                                         const uint16_t& num_bytes) {
  uint8_t mem, buf;
  for (int ii = 0; ii < num_bytes; ii++) {
    mem = memory_buffer[ii];
    buf = binary_buffer[ii];
    if (mem != buf) {
      Schmi::Error err = {"CheckBytes", "Bytes do not match", ii};
      err_->Init(err);
      err_->DisplayAndDie();
      return 0;
    }
  }
  return 1;
}

uint16_t FlashLoader::CheckNumBytesToWrite(const uint32_t& bytes_left) {
  uint16_t num_bytes_to_write = 0;

  if (bytes_left < MAX_WRITE_SIZE) {
    num_bytes_to_write = bytes_left;
  } else {
    num_bytes_to_write = MAX_WRITE_SIZE;
  }

  return num_bytes_to_write;
}

void FlashLoader::UpdateBinaryBytesData(BinaryBytesData& binary_bytes_data,
                                        const uint16_t& num_bytes) {
  binary_bytes_data.current_byte_pos += num_bytes;
  binary_bytes_data.current_memory_address += num_bytes;
  binary_bytes_data.bytes_left -= num_bytes;

  return;
}
}  // namespace Schmi

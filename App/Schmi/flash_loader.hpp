#ifndef SCHMI_FLASH_LOADER_HPP
#define SCHMI_FLASH_LOADER_HPP

#include "Schmi/binary_file_interface.hpp"
#include "Schmi/error_handler_interface.hpp"
#include "Schmi/loading_bar_interface.hpp"
#include "Schmi/serial_interface.hpp"
#include "Schmi/stm32.hpp"

namespace Schmi {

struct BinaryBytesData {
  uint32_t current_byte_pos;
  uint32_t current_memory_address;
  uint32_t bytes_left;
};

const uint16_t MAX_NUM_PAGES_TO_ERASE = 512;

class FlashLoader {
 public:
  const uint16_t MAX_WRITE_SIZE = 256;

  FlashLoader(SerialInterface* ser, BinaryFileInterface* bin, ErrorHandlerInterface* err,
              LoadingBarInterface* bar)
      : ser_(ser), bin_(bin), err_(err), bar_(bar) {
    stm32_ = new Stm32(*ser_, *err_);
  };
  ~FlashLoader() { delete stm32_; };

  void Init();

  bool InitUsart();

  // For global erase
  bool Flash(bool init_usart = true, bool global_erase = false);

  // For erasing only certain pages
  bool Flash(uint16_t* page_codes, const uint16_t& num_of_pages, bool init_usart = true);

 private:
  const uint32_t START_ADDRESS_ = 0x08000000;
  const uint16_t PAGE_SIZE_ = 2000;

  SerialInterface* ser_;
  BinaryFileInterface* bin_;
  ErrorHandlerInterface* err_;
  LoadingBarInterface* bar_;
  Stm32* stm32_;

  uint32_t total_num_bytes_ = 0;

  uint16_t pages_codes_buffer[MAX_NUM_PAGES_TO_ERASE];

  uint16_t GetPagesCodesFromBinary();

  bool FlashBytes();
  bool CheckMemory();
  bool CompareBinaryAndMemory(uint8_t* memory_buffer, uint8_t* binary_buffer,
                              const uint16_t& num_bytes);

  uint16_t CheckNumBytesToWrite(const uint64_t& bytes_left);

  void UpdateBinaryBytesData(BinaryBytesData& binary_bytes_data, const uint16_t& num_bytes);
};
}  // namespace Schmi

#endif  // SCHMI_FLASH_LOADER_HPP

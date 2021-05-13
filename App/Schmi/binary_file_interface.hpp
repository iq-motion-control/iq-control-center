#ifndef SCHMI_BINARY_FILE_INTERFACE_HPP
#define SCHMI_BINARY_FILE_INTERFACE_HPP

#include <cstdint>

namespace Schmi {

struct BytesData {
  uint32_t num_bytes;
  uint32_t starting_byte;
};

class BinaryFileInterface {
 public:
  virtual ~BinaryFileInterface(){};

  virtual void Init() = 0;
  virtual uint64_t GetBinaryFileSize() = 0;
  virtual void GetBytesArray(uint8_t* bytes, const BytesData& bytes_data) = 0;
};
}  // namespace Schmi

#endif  // SCHMI_BINARY_FILE_INTERFACE
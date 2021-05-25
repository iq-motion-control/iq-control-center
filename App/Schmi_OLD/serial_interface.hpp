#ifndef SCHMI_SERIAL_INTERFACE_HPP
#define SCHMI_SERIAL_INTERFACE_HPP

#include <stdint.h>
#include <cstddef>

namespace Schmi {

class SerialInterface {
 public:
  virtual ~SerialInterface(){};

  virtual void Init() = 0;
  virtual int Write(uint8_t* buffer, const uint16_t& buffer_length) = 0;
  virtual int Read(uint8_t* buffer, const uint16_t& num_bytes, const uint16_t& timeout_ms) = 0;
};
}  // namespace Schmi

#endif  // SCHMI_SERIAL_INTERFACE_HPP
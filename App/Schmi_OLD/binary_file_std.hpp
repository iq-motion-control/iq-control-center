#ifndef SCHMI_BINARY_FILE_STD_HPP
#define SCHMI_BINARY_FILE_STD_HPP

#include "Schmi/binary_file_interface.hpp"

#include "Schmi/std_exception.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace Schmi {

class BinaryFileStd : public BinaryFileInterface {
 public:
  BinaryFileStd(std::string binary_file_name) : binary_file_name_(binary_file_name){};
  ~BinaryFileStd(){};

  void Init() override;
  uint64_t GetBinaryFileSize() override { return binary_file_size_; };
  void GetBytesArray(uint8_t* bytes, const BytesData& bytes_data) override;

 private:
  std::string binary_file_name_;
  uint64_t binary_file_size_ = 0;
  std::vector<uint8_t> bytes_;

  uint64_t FindFileSize(std::ifstream& file);
  std::vector<uint8_t> ReadFile(std::ifstream& file);
};
}  // namespace Schmi
#endif  // SCHMI_BINARY_FILE_STD
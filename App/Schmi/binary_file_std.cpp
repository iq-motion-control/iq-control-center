#include "Schmi/binary_file_std.hpp"

namespace Schmi {

void BinaryFileStd::Init() {
  std::ifstream input_file;
  try {
    input_file.open(binary_file_name_, std::ios::binary | std::ios::ate);
    if (input_file.fail()) {
      throw StdException("Fail opening file, check file name/path");
    }

    binary_file_size_ = FindFileSize(input_file);

    bytes_ = ReadFile(input_file);

  } catch (const StdException& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    exit(EXIT_FAILURE);
  }
  return;
}

void BinaryFileStd::GetBytesArray(uint8_t* bytes, const BytesData& bytes_data) {
  std::vector<uint8_t>::const_iterator first = bytes_.begin() + bytes_data.starting_byte;
  std::vector<uint8_t>::const_iterator last = first + bytes_data.num_bytes;

  std::copy(first, last, bytes);

  return;
}

uint64_t BinaryFileStd::FindFileSize(std::ifstream& file) {
  file.seekg(0, file.end);
  uint64_t file_size = file.tellg();
  file.seekg(0, file.beg);

  return file_size;
}

std::vector<uint8_t> BinaryFileStd::ReadFile(std::ifstream& file) {
  char raw_data[binary_file_size_];

  file.read(raw_data, binary_file_size_);
  std::vector<uint8_t> bytes(raw_data, raw_data + binary_file_size_);

  return bytes;
}
}  // namespace Schmi
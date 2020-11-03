#include "Schmi/loading_bar_std.hpp"

namespace Schmi {

void LoadingBarStd::StartLoadingBar(const uint64_t& total_num_bytes) {
  total_num_bytes_ = total_num_bytes;
  std::cout << "Flashing controller\n";

  return;
}

void LoadingBarStd::StartCheckingLoadingBar(const uint64_t& total_num_bytes) {
  total_num_bytes_ = total_num_bytes;
  std::cout << "Checking controller\n";

  return;
}

void LoadingBarStd::UpdateLoadingBar(const uint64_t& bytes_left) {
  float progress = 1.0 - ((float)bytes_left / (float)total_num_bytes_);

  DrawBar(progress);

  return;
}

void LoadingBarStd::DrawBar(const float& progress) {
  uint8_t bar_width = 70;

  std::cout << "[";
  uint8_t pos = bar_width * progress;
  for (int i = 0; i < bar_width; ++i) {
    if (i < pos)
      std::cout << "=";
    else if (i == pos)
      std::cout << ">";
    else
      std::cout << " ";
  }
  std::cout << "] " << int(progress * 100.0) << " %\r";
  std::cout.flush();

  return;
}

void LoadingBarStd::EndLoadingBar() {
  std::cout << "\nSUCCESS\n";

  return;
}
}  // namespace Schmi

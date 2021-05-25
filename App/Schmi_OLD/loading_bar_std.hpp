#ifndef SCHMI_LOADING_BAR_STD_HPP
#define SCHMI_LOADING_BAR_STD_HPP

#include "Schmi/loading_bar_interface.hpp"

#include <iostream>

namespace Schmi {

class LoadingBarStd : public LoadingBarInterface {
 public:
  LoadingBarStd(){};
  ~LoadingBarStd(){};

  void StartLoadingBar(const uint64_t& total_num_bytes) override;
  void StartCheckingLoadingBar(const uint64_t& total_num_bytes) override;
  void UpdateLoadingBar(const uint64_t& bytes_left) override;
  void EndLoadingBar() override;

 private:
  uint64_t total_num_bytes_ = 0;

  void DrawBar(const float& progress);
};
}  // namespace Schmi

#endif  // SCHMI_LOADING_BAR_STD_HPP
#ifndef SCHMI_LOADING_BAR_INTERFACE_HPP
#define SCHMI_LOADING_BAR_INTERFACE_HPP

#include <stdint.h>

namespace Schmi {

class LoadingBarInterface {
 public:
  virtual ~LoadingBarInterface(){};

  virtual void StartLoadingBar(const uint64_t& total_num_bytes) = 0;
  virtual void StartCheckingLoadingBar(const uint64_t& total_num_bytes) = 0;
  virtual void UpdateLoadingBar(const uint64_t& bytes_left) = 0;
  virtual void EndLoadingBar() = 0;
};
}  // namespace Schmi

#endif  // SCHMI_LOADING_BAR_INTERFACE_HPP
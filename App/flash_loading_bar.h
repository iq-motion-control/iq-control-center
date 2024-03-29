#ifndef FLASHLOADINGBAR_H
#define FLASHLOADINGBAR_H

#include <QProgressBar>

#include "iq_flasher/include/Schmi/loading_bar_interface.hpp"
#include "main.h"

class FlashLoadingBar : public Schmi::LoadingBarInterface {
 public:
  FlashLoadingBar(QProgressBar* flash_progress_bar);
  ~FlashLoadingBar(){};

  void StartLoadingBar(const uint64_t& total_num_bytes) override;
  void StartCheckingLoadingBar(const uint64_t& total_num_bytes) override;
  void UpdateLoadingBar(const uint64_t& bytes_left) override;
  void EndLoadingBar() override;

 private:
  bool started_check_ = false;
  QProgressBar* flash_progress_bar_;
  uint64_t total_num_bytes_ = 0;
};

#endif  // FLASHLOADINGBAR_H

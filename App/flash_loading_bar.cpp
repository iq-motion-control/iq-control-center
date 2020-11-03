#include "flash_loading_bar.h"

FlashLoadingBar::FlashLoadingBar(QProgressBar* flash_progress_bar)
    : flash_progress_bar_(flash_progress_bar) {
  flash_progress_bar_->setMinimum(0);
}

void FlashLoadingBar::StartLoadingBar(const uint64_t& total_num_bytes) {
  iv.label_message->setText("Starting flash");
  total_num_bytes_ = total_num_bytes;
  flash_progress_bar_->reset();
  flash_progress_bar_->setMaximum(total_num_bytes);
}

void FlashLoadingBar::StartCheckingLoadingBar(const uint64_t& total_num_bytes) {
  iv.label_message->setText("Starting flash check");
  total_num_bytes_ = total_num_bytes;
  flash_progress_bar_->reset();
  flash_progress_bar_->setMaximum(total_num_bytes);
}

void FlashLoadingBar::UpdateLoadingBar(const uint64_t& bytes_left) {
  qint64 progress = total_num_bytes_ - bytes_left;
  flash_progress_bar_->setValue(progress);
};

void FlashLoadingBar::EndLoadingBar() { iv.label_message->setText("FLASHING SUCCESFULL"); };

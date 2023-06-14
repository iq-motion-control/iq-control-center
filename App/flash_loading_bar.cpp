#include "flash_loading_bar.h"

FlashLoadingBar::FlashLoadingBar(QProgressBar* flash_progress_bar)
    : flash_progress_bar_(flash_progress_bar) {
  flash_progress_bar_->setMinimum(0);
}

void FlashLoadingBar::StartLoadingBar(const uint64_t& total_num_bytes) {
  started_check_ = false;
  QString start_flash("Starting flash");
  iv.label_message->setText(start_flash);
  iv.pcon->AddToLog(start_flash.toLower());
  total_num_bytes_ = total_num_bytes * 2;  // flashing + check
  flash_progress_bar_->reset();
  flash_progress_bar_->setMaximum(total_num_bytes_);
}

void FlashLoadingBar::StartCheckingLoadingBar(const uint64_t& total_num_bytes) {
  // We don't reset the bar because we are using one bar for both flash and check
  started_check_ = true;
  QString label_text("Starting flash check");
  iv.label_message->setText(label_text);
  iv.pcon->AddToLog(label_text.toLower());
}

void FlashLoadingBar::UpdateLoadingBar(const uint64_t& bytes_left) {
  qint64 progress;
  if (started_check_) {
    progress = total_num_bytes_ - bytes_left;
  } else {
    progress = (total_num_bytes_ / 2) - bytes_left;
  }
  flash_progress_bar_->setValue(progress);
  QCoreApplication::processEvents();
};

void FlashLoadingBar::EndLoadingBar() {
  iv.label_message->setText("Flashing Successful");
  started_check_ = false;
};

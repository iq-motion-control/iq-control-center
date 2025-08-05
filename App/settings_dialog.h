#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include "app_settings.h"

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(AppSettings* settings, QWidget* parent = nullptr);

 private slots:
  void accept() override;

 private:
  AppSettings* appSettings;
  QCheckBox* showAppUpdateMessageBox;
  QCheckBox* showAdvancedFlashingOptions;
  QCheckBox* m_autoSaveCheck;
};

#endif // SETTINGSDIALOG_H

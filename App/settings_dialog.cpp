#include "settings_dialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>

SettingsDialog::SettingsDialog(AppSettings* settings, QWidget* parent)
    : QDialog(parent), appSettings(settings) {
  setWindowTitle("Application Settings");

  showAppUpdateMessageBox = new QCheckBox("Enable new version popup notification");
  showAppUpdateMessageBox->setChecked(appSettings->get("show_update_message_box", true).toBool());
  showAdvancedFlashingOptions = new QCheckBox("Advanced flashing options");
  showAdvancedFlashingOptions->setChecked(appSettings->get("show_advanced_flashing_options", false).toBool());

  QVBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(showAppUpdateMessageBox);
  layout->addWidget(showAdvancedFlashingOptions);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
  layout->addWidget(buttonBox);

  setLayout(layout);
}

void SettingsDialog::accept() {
  appSettings->set("show_update_message_box", showAppUpdateMessageBox->isChecked());
  appSettings->set("show_advanced_flashing_options", showAdvancedFlashingOptions->isChecked());
  iv.pcon->AddToLog("Setting and saving application settings.");
  appSettings->save();
  QDialog::accept();
}

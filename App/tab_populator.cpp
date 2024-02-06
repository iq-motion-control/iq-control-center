/*
    Copyright 2018 - 2019 IQ Motion Control   	raf@iq-control.com

    This file is part of IQ Control Center.

    IQ Control Center is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    IQ Control Center is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "tab_populator.h"

TabPopulator::TabPopulator(Ui::MainWindow *ui, ResourceFileHandler * resource_file_handler, std::map<std::string, std::shared_ptr<Tab>> *tab_map)
    : ui_(ui), resource_file_handler_(resource_file_handler), tab_map_(tab_map) {}

void TabPopulator::PopulateTabs(int hardware_type, int hardware_major_version, int electronics_type,
                                int electronics_major_version, int firmware_style, int firmware_value) {

  //FRED NOTE: Debugging stuff
  qInfo("TAB POPULATOR\n-------------------------------------------------\n");
  qInfo(qPrintable("Hardware Type: " + QString::number(hardware_type)));
  qInfo(qPrintable("Hardware Major Version: " + QString::number(hardware_major_version)));
  qInfo(qPrintable("Electronics Type: " + QString::number(electronics_type)));
  qInfo(qPrintable("Electronics Major Version: " + QString::number(electronics_major_version)));
  qInfo(qPrintable("Firmware Style: " + QString::number(firmware_style)));
  qInfo(qPrintable("Firmware Value: " + QString::number(firmware_value)));

  resource_file_handler_->LoadResourceFile(hardware_type, hardware_major_version, electronics_type, electronics_major_version, firmware_style);

//  Fred Notes: This may or may not be able to stay the same, might need two versions for legacy vs. new?
  DisplayFirmwareHardwareName();

  //Fred Notes: This can probably stay the same as well if we set firmware_styles correctly
  CheckMinFirmwareBuildNumber(firmware_value);

  //Fred Notes: Ideally this can be the same as well? I think it can be, we'll see
  CreateTabFrames();
}

void TabPopulator::DisplayFirmwareHardwareName() {
  QString firmware_display_name = QString::fromStdString(resource_file_handler_->firmware_name_).split("-")[0];
  QString harwdware_display_name = QString::fromStdString(resource_file_handler_->hardware_name_);
  ui_->label_firmware_name->setText(firmware_display_name);
  ui_->label_hardware_name->setText(harwdware_display_name);

  iv.pcon->logging_active_ = true;
  iv.pcon->AddToLog("module connected has firmware style: " + firmware_display_name);
  iv.pcon->AddToLog("module connected has hardware style: " + harwdware_display_name + "\n");
  iv.pcon->logging_active_ = false;
}

void TabPopulator::CheckMinFirmwareBuildNumber(const int &firmware_build_number) {
  //firmware_build_number holds the raw 32 bits of firmware information
  //(style - 12 bits) | (major - 6) | (minor - 7) | (patch - 7)
  int firmware_build_major = (firmware_build_number & MAJOR_VERSION_MASK) >> MAJOR_VERSION_SHIFT;
  int firmware_build_minor = (firmware_build_number & MINOR_VERSION_MASK) >> MINOR_VERSION_SHIFT;
  int firmware_build_patch = firmware_build_number & PATCH_VERSION_MASK;

  //Check each of the values, if major or minor are above the minimum version, assume we are good to go
  if(firmware_build_major > resource_file_handler_->minimum_firmware_major_){
        return;
  }else if((firmware_build_major == resource_file_handler_->minimum_firmware_major_) &&
            firmware_build_minor > resource_file_handler_->minimum_firmware_minor_){
        return;
  }

  //If the major and minor are less than than or equal to their build go here
  if(firmware_build_major < resource_file_handler_->minimum_firmware_major_){
      DisplayUpdateFirmwareWarning();
  }else if(firmware_build_minor < resource_file_handler_->minimum_firmware_minor_){
      DisplayUpdateFirmwareWarning();
  }else if(firmware_build_patch < resource_file_handler_->minimum_firmware_patch_){
      DisplayUpdateFirmwareWarning();
  }

  return;
}

void TabPopulator::DisplayUpdateFirmwareWarning() {
  QMessageBox msgBox;
  msgBox.setWindowTitle("WARNING!");
  msgBox.setText(
      "Your module's firmware version is below the minimum set by this version of the IQ Control "
      "Center.\nSome features may not work properly, please visit our website for the latest "
      "available firmware.\nhttp://iq-control.com/support");
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();
  return;
}

void TabPopulator::CreateTabFrames() {
  std::map<QWidget *, std::vector<std::string>> tab_widget_firmware_files =
      LinkTabWidgetAndFirmwareFiles();

  tab_map_->clear();
  for (std::pair<QWidget *, std::vector<std::string>> tab_firmware : tab_widget_firmware_files) {
    std::shared_ptr<Tab> tab =
        std::make_shared<Tab>(tab_firmware.first, iv.pcon->GetObjId(), tab_firmware.second);
    tab->CreateFrames();
    UpdateTabMap(tab, tab_firmware.second[1]);
  }
}

void TabPopulator::UpdateTabMap(std::shared_ptr<Tab> &tab, std::string &tab_name) {
  tab_map_->insert(std::pair<std::string, std::shared_ptr<Tab>>(tab_name, tab));
  return;
}

std::map<QWidget *, std::vector<std::string>> TabPopulator::LinkTabWidgetAndFirmwareFiles() {
  std::map<QWidget *, std::vector<std::string>> tab_widget_firmware_files;
  tab_widget_firmware_files[ui_->general_scroll_area] = {"/Resources/Tabs/general/",
                                                         "general_" + resource_file_handler_->firmware_name_ + ".json"};
  tab_widget_firmware_files[ui_->tuning_scroll_area] = {"/Resources/Tabs/tuning/",
                                                        "tuning_" + resource_file_handler_->firmware_name_ + ".json"};
  tab_widget_firmware_files[ui_->advanced_scroll_area] = {"/Resources/Tabs/advanced/",
                                                          "advanced_" + resource_file_handler_->firmware_name_ + ".json"};
  tab_widget_firmware_files[ui_->testing_scroll_area] = {"/Resources/Tabs/testing/",
                                                         "testing_" + resource_file_handler_->firmware_name_ + ".json"};
  return tab_widget_firmware_files;
}

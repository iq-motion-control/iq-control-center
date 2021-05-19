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

TabPopulator::TabPopulator(Ui::MainWindow *ui, std::map<std::string, std::shared_ptr<Tab>> *tab_map)
    : ui_(ui), tab_map_(tab_map) {}

void TabPopulator::PopulateTabs(int hardware_type, int firmware_style, int firmware_build_number) {
  LoadFirmwareStylesFromHardwareType(hardware_type);
  FindFirmwareIndex(firmware_style);
  GetAndDisplayFirmwareHardwareName();
  CheckMinFirmwareBuildNumber(firmware_build_number);
  CreateTabFrames();
}

void TabPopulator::LoadFirmwareStylesFromHardwareType(const int &hardware_type) {
  QString current_path = QCoreApplication::applicationDirPath();
  QString hardware_type_file_path =
      current_path + "/Resources/Firmware/" + QString::number(hardware_type) + ".json";
  firmware_styles_ = OpenAndLoadJsonFile(hardware_type_file_path);
}

Json::Value TabPopulator::OpenAndLoadJsonFile(const QString &file_path) {
  Json::Value my_json_value;
  QFile my_file(file_path);
  if (my_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    my_json_value = LoadJsonFile(my_file);
    my_file.close();
  } else {
    throw QString("Resource File Missing, Update App");
  }
  return my_json_value;
}

Json::Value TabPopulator::LoadJsonFile(QFile &my_file) {
  std::istringstream iss(QTextStream(&my_file).readAll().toStdString());
  std::string errs;
  Json::Value my_json_value;
  Json::parseFromStream(json_.rbuilder, iss, &my_json_value, &errs);
  return my_json_value;
}

void TabPopulator::FindFirmwareIndex(const int &firmware_style) {
  uint32_t num_of_firmware_styles = firmware_styles_.size();
  for (uint32_t ii = 0; ii < num_of_firmware_styles; ++ii) {
    if (firmware_style == firmware_styles_[ii]["style"].asInt()) {
      firmware_index_ = ii;
      return;
    }
  }
  throw QString("Firmware Style Not Handled");
}

void TabPopulator::GetAndDisplayFirmwareHardwareName() {
  GetFirmwareHardwareName();
  DisplayFirmwareHardwareName();
  return;
}

void TabPopulator::GetFirmwareHardwareName() {
  firmware_name_ = firmware_styles_[firmware_index_]["name"].asString();
  hardware_name_ = firmware_styles_[0]["hardware_name"].asString();
  return;
}

void TabPopulator::DisplayFirmwareHardwareName() {
  QString firmware_display_name = QString::fromStdString(firmware_name_).split("-")[0];
  QString harwdware_display_name = QString::fromStdString(hardware_name_);
  ui_->label_firmware_name->setText(firmware_display_name);
  ui_->label_hardware_name->setText(harwdware_display_name);
}

void TabPopulator::CheckMinFirmwareBuildNumber(const int &firmware_build_number) {
  if (firmware_build_number < firmware_styles_[firmware_index_]["min_build_number"].asInt()) {
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
                                                         "general_" + firmware_name_ + ".json"};
  tab_widget_firmware_files[ui_->tuning_scroll_area] = {"/Resources/Tabs/tuning/",
                                                        "tuning_" + firmware_name_ + ".json"};
  tab_widget_firmware_files[ui_->advanced_scroll_area] = {"/Resources/Tabs/advanced/",
                                                          "advanced_" + firmware_name_ + ".json"};
  tab_widget_firmware_files[ui_->testing_scroll_area] = {"/Resources/Tabs/testing/",
                                                         "testing_" + firmware_name_ + ".json"};
  return tab_widget_firmware_files;
}

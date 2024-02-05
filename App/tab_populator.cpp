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

void TabPopulator::PopulateTabs(int hardware_type, int hardware_major_version, int electronics_type,
                                int electronics_major_version, int firmware_style, int firmware_value) {

  //FRED NOTE: Debugging stuff
  qInfo(qPrintable("Hardware Type: " + QString::number(hardware_type)));
  qInfo(qPrintable("Hardware Major Version: " + QString::number(hardware_major_version)));
  qInfo(qPrintable("Electronics Type: " + QString::number(electronics_type)));
  qInfo(qPrintable("Electronics Major Version: " + QString::number(electronics_major_version)));
  qInfo(qPrintable("Firmware Style: " + QString::number(firmware_style)));
  qInfo(qPrintable("Firmware Value: " + QString::number(firmware_value)));

  //Fred Note: Ok, so this guy takes just the hardware type, and from there loads the JSON file.
  //That part is still correct with my current decision to keep the filenames to be just the hardware type,
  //so I guess this guy is still useful, BUT the way it loads in those firmware styles isn't really going
  //to work anymore.
  //So I think this place is the most important one to handle the split and to set firmware_styles_ appropriately
  LoadFirmwareStyles(hardware_type, hardware_major_version, electronics_type, electronics_major_version);


  //Fred Notes: If we set firmware_styles correctly, than this could stay the same. So far this seems good
  FindFirmwareIndex(firmware_style);

//  Fred Notes: This may or may not be able to stay the same, might need two versions for legacy vs. new?
  GetAndDisplayFirmwareHardwareName();

  //Fred Notes: This can probably stay the same as well if we set firmware_styles correctly
  CheckMinFirmwareBuildNumber(firmware_value);

  //Fred Notes: Ideally this can be the same as well? I think it can be, we'll see
  CreateTabFrames();
}

bool TabPopulator::IsLegacyJsonFile(Json::Value json_file){
  //A very easy way to tell the legacy files from the new ones is that the legacy ones are an array at the top level,
  //while the new ones are not. So if the top level of the file is an array, it must be legacy.
  return json_file_.isArray();
}

Json::Value TabPopulator::ExtractModuleConfigurationFromNewStyleFile(Json::Value json_file, const int &hardware_major_version,
                                                                const int& electronics_type, const int& electronics_major_version){
    //We need to search through the list of possible module configurations and find one that matches our full module definition
    Json::Value possible_module_configurations = json_file["possible_module_configurations"];
    uint8_t number_of_possible_module_configurations = possible_module_configurations.size();

    for(uint8_t configuration = 0; configuration < number_of_possible_module_configurations; configuration++){
        //Extract all the key info from each configuration, check if it matches
        int configuration_hardware_major_version = possible_module_configurations[configuration]["hardware_major_version"].asInt();
        int configuration_electronics_type = possible_module_configurations[configuration]["electronics_type"].asInt();
        int configuration_electronics_major_version = possible_module_configurations[configuration]["electronics_major_version"].asInt();

        //If we have a match, return the configuration
        if((configuration_hardware_major_version == hardware_major_version) &&
           (configuration_electronics_type == electronics_type) &&
           (configuration_electronics_major_version == electronics_major_version)){


           qInfo("Found module configuration");
           return possible_module_configurations[configuration];
        }
    }

    //Fred Note: Is it ok that I don't return anything here? Need to test out how this fails to make sure its graceful
    throw QString("Correct configuration could not be found for this module, update app.");
//    return Json::Value::null;
}

void TabPopulator::LoadFirmwareStyles(const int &hardware_type, const int &hardware_major_version,
                                      const int& electronics_type, const int& electronics_major_version) {
  QString current_path = QCoreApplication::applicationDirPath();
  QString hardware_type_file_path =
      current_path + "/Resources/Firmware/" + QString::number(hardware_type) + ".json";

  //Fred Note: We grab the JSON file here, but properly extracting the styles takes some more thought now
  //Fred Note: This may not need to be class-scoped, see how things shake out
  json_file_ = OpenAndLoadJsonFile(hardware_type_file_path);

  //Fred Notes: We may be able to tell new from old files quite easily by just checking for a top level array
  if(IsLegacyJsonFile(json_file_)){
      //This is a legacy style file, we can just load up the firmware styles directly out of it
      qInfo("Legacy file");
      firmware_styles_ = json_file_;

      //Easier to grab the hardware name now, since it differs between legacy and new style files
      hardware_name_ = firmware_styles_[0]["hardware_name"].asString();
  }else{
      qInfo("New style file");
      //Need to pull the appropriate list of firmware styles out of this thing based on the hardware major, electronics type, and electronics major.
      Json::Value module_configuration = ExtractModuleConfigurationFromNewStyleFile(json_file_, hardware_major_version, electronics_type, electronics_major_version);
      qInfo(qPrintable(module_configuration["hardware_name"].asCString()));

      firmware_styles_ = module_configuration["styles"];
      hardware_name_ = module_configuration["hardware_name"].asString();
  }

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
      qInfo(qPrintable("Found firmware index: "+QString::number(firmware_index_)));
      return;
    }
  }
  throw QString("Firmware Style Not Handled");
}

void TabPopulator::GetAndDisplayFirmwareHardwareName() {
  GetFirmwareName();
  DisplayFirmwareHardwareName();
  return;
}

void TabPopulator::GetFirmwareName() {
  firmware_name_ = firmware_styles_[firmware_index_]["name"].asString();
//  hardware_name_ = firmware_styles_[0]["hardware_name"].asString();
  return;
}

void TabPopulator::DisplayFirmwareHardwareName() {
  QString firmware_display_name = QString::fromStdString(firmware_name_).split("-")[0];
  QString harwdware_display_name = QString::fromStdString(hardware_name_);
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
  if(firmware_build_major > firmware_styles_[firmware_index_]["min_major_build"].asInt()){
        return;
  }else if((firmware_build_major == firmware_styles_[firmware_index_]["min_major_build"].asInt()) &&
            firmware_build_minor > firmware_styles_[firmware_index_]["min_minor_build"].asInt()){
        return;
  }

  //If the major and minor are less than than or equal to their build go here
  if(firmware_build_major < firmware_styles_[firmware_index_]["min_major_build"].asInt()){
      DisplayUpdateFirmwareWarning();
  }else if(firmware_build_minor < firmware_styles_[firmware_index_]["min_minor_build"].asInt()){
      DisplayUpdateFirmwareWarning();
  }else if(firmware_build_patch < firmware_styles_[firmware_index_]["min_patch_build"].asInt()){
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

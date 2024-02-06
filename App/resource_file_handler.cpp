#include "resource_file_handler.h"

ResourceFileHandler::ResourceFileHandler(){
    //Reset everything
    ReleaseResourceFile();
}


//Two versions of this to support darn recovery stuff that just wants to get hardware names
void ResourceFileHandler::LoadResourceFile(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version){
    qInfo("\nRESOURCE FILE HANDLER\n-------------------------------------------------");

    QString current_path = QCoreApplication::applicationDirPath();
    QString hardware_type_file_path =
        current_path + "/Resources/Firmware/" + QString::number(hardware_type) + ".json";

    json_file_ = OpenAndLoadJsonFile(hardware_type_file_path);

    loaded_hardware_type_ = hardware_type;
    loaded_hardware_major_version_ = hardware_major_version;
    loaded_electronics_type_ = electronics_type;
    loaded_electronics_major_version_ = electronics_major_version;

    //Fred Notes: We may be able to tell new from old files quite easily by just checking for a top level array
    if(IsLegacyJsonFile(json_file_)){
        //This is a legacy style file, we can just load up the firmware styles directly out of it
        qInfo("Legacy file");
        firmware_styles_ = json_file_;

        //Hardware name is also just sitting at the top level
        hardware_name_ = firmware_styles_[0]["hardware_name"].asString();

    }else{
        qInfo("New style file");
        //Need to pull the appropriate list of firmware styles out of this thing based on the hardware major, electronics type, and electronics major.
        Json::Value module_configuration = ExtractModuleConfigurationFromNewStyleFile(json_file_, hardware_major_version, electronics_type, electronics_major_version);
        qInfo(qPrintable(module_configuration["hardware_name"].asCString()));

        firmware_styles_ = module_configuration["styles"];
        hardware_name_ = module_configuration["hardware_name"].asString();
    }

    hardware_information_loaded_ = true;
}

//Finds and loads the appropriate resource file and fills up all of our resource file information
void ResourceFileHandler::LoadResourceFile(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version, const int& firmware_style){
    LoadResourceFile(hardware_type, hardware_major_version, electronics_type, electronics_major_version);

    loaded_firmware_style_ = firmware_style;

    //Fred Note: Having the style in here may complicate things, we'll see. May need to separate out the loading of firmware stuff from the initial resource file load?
    FindFirmwareIndex(firmware_style);

    //These can be pulled out of the firmware styles the same way between legacy and new file types
    minimum_firmware_major_ = firmware_styles_[firmware_index_]["min_major_build"].asInt();
    minimum_firmware_minor_ = firmware_styles_[firmware_index_]["min_minor_build"].asInt();
    minimum_firmware_patch_ = firmware_styles_[firmware_index_]["min_patch_build"].asInt();

    firmware_name_ = firmware_styles_[firmware_index_]["name"].asString();

    firmware_information_loaded_ = true;
}

void ResourceFileHandler::ReleaseResourceFile(){
    firmware_styles_ = Json::Value::null;
    firmware_index_ = -1;

    loaded_hardware_type_ = -1;
    loaded_hardware_major_version_ = -1;
    loaded_electronics_type_ = -1;
    loaded_electronics_major_version_ = -1;
    loaded_firmware_style_ = -1;

    minimum_firmware_major_ = -1;
    minimum_firmware_minor_ = -1;
    minimum_firmware_patch_ = -1;

    std::string firmware_name = "";
    std::string hardware_name_ = "";

    hardware_information_loaded_ = false;
    firmware_information_loaded_ = false;
}

Json::Value  ResourceFileHandler::OpenAndLoadJsonFile(const QString &file_path){
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

Json::Value  ResourceFileHandler::LoadJsonFile(QFile &my_file){
    std::istringstream iss(QTextStream(&my_file).readAll().toStdString());
    std::string errs;
    Json::Value my_json_value;
    Json::parseFromStream(json_.rbuilder, iss, &my_json_value, &errs);
    return my_json_value;
}

bool  ResourceFileHandler::IsLegacyJsonFile(Json::Value json_file){
    //A very easy way to tell the legacy files from the new ones is that the legacy ones are an array at the top level,
    //while the new ones are not. So if the top level of the file is an array, it must be legacy.
    return json_file_.isArray();
}

Json::Value  ResourceFileHandler::ExtractModuleConfigurationFromNewStyleFile(Json::Value json_file, const int &hardware_major_version,
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
}

void  ResourceFileHandler::FindFirmwareIndex(const int &firmware_style){
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

#ifndef RESOURCE_FILE_HANDLER_H
#define RESOURCE_FILE_HANDLER_H

#include <QObject>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include <QMessageBox>
#include "IQ_api/json_cpp.hpp"

//This loads in and holds onto the information from a resource file in a convenient shared location. It doesn't perform any checks, or care about what is connected,
//it just loads up the information from the specified portion of a resource file for anyone's consumption.
class ResourceFileHandler : public QObject {
  Q_OBJECT
 private:
  //JsonCpp object for reading from the JSON
  JsonCpp json_;

  //The full contents of the JSON file
  Json::Value json_file_;

  //The firmware styles list from the resource file. This contains a list of acceptable styles and minimum firmware versions for each.
  //This list is the same between both legacy and new style files, but new style files may contain multiple of these lists so it must
  //be extracted slightly different from them.
  Json::Value firmware_styles_;

  //The index in the firmware_styles list that holds the style we want to load for
  uint32_t firmware_index_;

  // The path of the SessionResourceFiles directory in AppData. This is provided by mainwindow.
  QString appDataSessionResourcesPath;

  /**
   * @brief OpenAndLoadJsonFile Opens and returns the contents of the JSON file at the provided path
   * @param file_path The path to the JSON file to open
   * @return The contents of the JSON file
   */
  Json::Value OpenAndLoadJsonFile(const QString &file_path);

  /**
   * @brief LoadJsonFile Reads the contents of an opened QFile object pointing to a JSON file
   * @param my_file A QFile object that points to a JSON file
   * @return The contents of the JSON file
   */
  Json::Value LoadJsonFile(QFile &my_file);

  /**
   * @brief IsLegacyResourceFile Determines if the given resource file is a legacy file, meaning that it uses the older style that does not include major version or electronics information.
   * @param json_file The contents of the resource file
   * @return True if it is a legacy file, false otherwise.
   */
  bool IsLegacyResourceFile(Json::Value json_file);

  /**
   * @brief ExtractModuleConfigurationFromNewStyleFile Extracts the requested module configuration dictionary from a new style resource file. From this, all of the rest of the information on
   * the request module can be accessed.
   * @param json_file The contents of the resource file
   * @param hardware_major_version The hardware major version of the configuration to extract
   * @param electronics_type The electronics tpe of the configuration to extract
   * @param electronics_major_version The electronics major version of the configuration to extract
   * @return The module configuration dictionary. Throws an exception if it cannot find the right configuration.
   */
  Json::Value ExtractModuleConfigurationFromNewStyleFile(Json::Value json_file, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);

  /**
   * @brief FindFirmwareIndex Finds the index of the target firmware style in our list of firmware styles, and sets the firmware_index_ if found.
   * @param firmware_style The firmware_style to search for
   * @return None
   */
  void FindFirmwareIndex(const int &firmware_style);

 public:
  //These provide a record of the what sort of resource file we have loaded.
  //If you want to check the details of what we loaded, refer to these
  int loaded_hardware_type_;
  int loaded_hardware_major_version_;
  int loaded_electronics_type_;
  int loaded_electronics_major_version_;
  int loaded_firmware_style_;

  //You can just load hardware information, or you can specify the style as well
  //and load the full firmware information. These booleans can tell you at a glance
  //what subset of the information you have. Loading just the hardware information
  //is useful in certain scenarios, especially zip file checking and recovery.
  bool hardware_information_loaded_;
  bool firmware_information_loaded_;


  //Name used for the hardware, loaded from the resource file
  std::string hardware_name_;

  //Name used for the firmware, loaded from the resource file
  std::string firmware_name_;

  //Firmware version information. loaded from the resoruce file
  int minimum_firmware_major_;
  int minimum_firmware_minor_;
  int minimum_firmware_patch_;

  ResourceFileHandler(QString appDataSessionResourcesPath);

  /**
   * @brief LoadConfigurationFromResourceFile Finds the appropriate resource file and loads the hardware information for the correct configuration into the ResourceFileHandler. This only loads hardware
   * information, not firmware information. If you want to know about firwmare information, use the other version of this function. This is useful for instances where we only care
   * about the hardware information, such as when flashing and recovering.
   * @param hardware_type The hardware type of the resource file to search for. This will be the name of the resource file, e.g. "28.json"
   * @param hardware_major_version The hardware major version of the configuration to load from the resource file.
   * @param electronics_type The electronics type of the configuration to load from the resource file.
   * @param electronics_major_version The electronics major version of the configuration to load from the resource file.
   * @return True if resource file is loaded, False otherwise
   */
  bool LoadConfigurationFromResourceFile(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);

  /**
   * @brief LoadConfigurationFromResourceFile Finds the appropriate resource file and loads both the hardware and firmware information for the correct configuration into the ResourceFileHandler.
   * @param hardware_type The hardware type of the resource file to search for. This will be the name of the resource file, e.g. "28.json"
   * @param hardware_major_version The hardware major version of the configuration to load from the resource file.
   * @param electronics_type The electronics type of the configuration to load from the resource file.
   * @param electronics_major_version The electronics major version of the configuration to load from the resource file.
   * @param firmware_style The firmware style of the configuration to load information from.
   * @return True if resource file is loaded, False otherwise
   */
  bool LoadConfigurationFromResourceFile(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version, const int& firmware_style);

  /**
   * @brief ReleaseResourceFile Clear all of the resource file and configuration data from this handler. Generally encouraged to do after using the information so
   * that old resource file information is not left in the handler.
   * @return None
   */
  void ReleaseResourceFile();


  /**
   * @brief DisplayInvalidStyleWarning Displays a warning message indicating that the resource file could not be properly loaded for the connected module.
   * Includes Hardware, Electronics, and Firmware Styles of the connected module.
   * @param errorMessage The error message to display to the user
   * @param hardwareStyle The detected Hardware Style of the connected module
   * @param electronicsStyle The detected Electronics Style of the connected module
   * @param firmwareStyle The detected Firmware Style of the connected module
   * @return None
   */
  void DisplayInvalidStyleWarning(const QString &errorMessage, const int &hardwareStyle, const int &electronicsStyle, const int &firmwareStyle);

};

#endif // RESOURCE_FILE_HANDLER_H

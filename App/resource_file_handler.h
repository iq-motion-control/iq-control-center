#ifndef RESOURCE_FILE_HANDLER_H
#define RESOURCE_FILE_HANDLER_H

#include <QObject>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>
#include "IQ_api/json_cpp.hpp"

//This loads in and holds onto the information from a resource file in a convenient shared location. It doesn't perform any checks, or care about what is connected
class ResourceFileHandler : public QObject {
  Q_OBJECT
 private:
  JsonCpp json_;

  //The full contents of the JSON file
  Json::Value json_file_;

  Json::Value firmware_styles_;
  uint32_t firmware_index_;

  Json::Value OpenAndLoadJsonFile(const QString &file_path);
  Json::Value LoadJsonFile(QFile &my_file);
  bool IsLegacyJsonFile(Json::Value json_file);
  Json::Value ExtractModuleConfigurationFromNewStyleFile(Json::Value json_file, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);
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

  //The information loaded from the resource file for later consumption.
  std::string hardware_name_;

  std::string firmware_name_;

  int minimum_firmware_major_;
  int minimum_firmware_minor_;
  int minimum_firmware_patch_;

  ResourceFileHandler();

  //Does not load firmware stuff, only hardware stuff
  void LoadResourceFile(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);

  //Finds and loads the appropriate resource file, filling in all the details of the resource file handler
  void LoadResourceFile(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version, const int& firmware_style);

  //Release the resource file we have right now, clearing out all of our data in it.
  void ReleaseResourceFile();
};

#endif // RESOURCE_FILE_HANDLER_H

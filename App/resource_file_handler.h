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
  //Fred Note: May not need these guys, just there for completeness at the moment
  int hardware_type_;
  int hardware_major_version_;
  int electronics_type_;
  int electronics_major_version_;
  int firmware_style_;

  int minimum_firmware_major_;
  int minimum_firmware_minor_;
  int minimum_firmware_patch_;

  std::string firmware_name_;
  std::string hardware_name_;

  //For keeping track if we have a resource file loaded already or not
  bool resource_file_loaded_;

  ResourceFileHandler();

  //Finds and loads the appropriate resource file, filling in all the details of the resource file handler
  void LoadResourceFile(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version, const int& firmware_style);

  //Release the resource file we have right now, clearing out all of our data in it.
  void ReleaseResourceFile();

  Json::Value GetFirmwareStyles();
  std::string GetFirmwareName();
  std::string GetHardwareName();

};

#endif // RESOURCE_FILE_HANDLER_H

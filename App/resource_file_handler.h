#ifndef RESOURCE_FILE_HANDLER_H
#define RESOURCE_FILE_HANDLER_H

#include <QObject>
#include <QFile>
#include "IQ_api/json_cpp.hpp"

class ResourceFileHandler : public QObject {
  Q_OBJECT
 private:
  JsonCpp json_;

  //The full contents of the JSON file
  Json::Value json_file_;

  Json::Value firmware_styles_;
  uint32_t firmware_index_;
  std::string firmware_name_;
  std::string hardware_name_;

  Json::Value OpenAndLoadJsonFile(const QString &file_path);
  Json::Value LoadJsonFile(QFile &my_file);
  bool IsLegacyJsonFile(Json::Value json_file);
  Json::Value ExtractModuleConfigurationFromNewStyleFile(Json::Value json_file, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);
  void LoadFirmwareStyles(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);
  void FindFirmwareIndex(const int &firmware_style);

 public:
  ResourceFileHandler();

  //Finds and loads the appropriate resource file
  void LoadResourceFile(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);

  Json::Value GetFirmwareStyles();
  std::string GetFirmwareName();
  std::string GetHardwareName();

};

#endif // RESOURCE_FILE_HANDLER_H

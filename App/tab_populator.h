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

#ifndef TAB_POPULATOR_HPP
#define TAB_POPULATOR_HPP

#include <QObject>
#include "IQ_api/json_cpp.hpp"
#include "tab.h"

#include <map>

class TabPopulator : public QObject {
  Q_OBJECT
 private:
  Ui::MainWindow *ui_;
  std::map<std::string, std::shared_ptr<Tab>> *tab_map_;
//  JsonCpp json_;
//  Json::Value firmware_styles_;
//  Json::Value json_file_;
//  uint32_t firmware_index_;
//  std::string firmware_name_;
//  std::string hardware_name_;

//  void LoadFirmwareStyles(const int &hardware_type, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);
//  Json::Value OpenAndLoadJsonFile(const QString &file_path);
//  Json::Value LoadJsonFile(QFile &my_file);
//  bool IsLegacyJsonFile(Json::Value json_file);
//  Json::Value ExtractModuleConfigurationFromNewStyleFile(Json::Value json_file, const int &hardware_major_version, const int& electronics_type, const int& electronics_major_version);

//  void FindFirmwareIndex(const int &firmware_style);

  void GetAndDisplayFirmwareHardwareName();
//  void GetFirmwareName();
  void DisplayFirmwareHardwareName();

  void CheckMinFirmwareBuildNumber(const int &firmware_build_number);
  void DisplayUpdateFirmwareWarning();

  void CreateTabFrames();
  std::map<QWidget *, std::vector<std::string>> LinkTabWidgetAndFirmwareFiles();
  void UpdateTabMap(std::shared_ptr<Tab> &tab, std::string &tab_name);

 public:
  TabPopulator(Ui::MainWindow *ui, std::map<std::string, std::shared_ptr<Tab>> *tab_map);
  QString GetHardwareType() { return QString::fromStdString(hardware_name_);}
 signals:

 public slots:
  void PopulateTabs(int hardware_type, int hardware_major_version, int electronics_type, int electronics_major_version, int firmware_style, int firmware_value);
};

#endif  // TAB_POPULATOR_HPP

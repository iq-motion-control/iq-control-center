/*
    Copyright 2018 - 2019 IQ Motion Control   	dskart11@gmail.com

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

#include "frame_variables.h"
#include <QDebug>

std::map<std::string, FrameVariables *> FrameVariablesFromJson(const std::string &file_name,
                                                               const std::string &folder_path,
                                                               bool using_custom_order) {
  JsonCpp json;
  Json::Value JSON;
  std::map<std::string, FrameVariables *> frame_variables_map;

  QString path = QString::fromStdString(folder_path + file_name);
  QFile myfile(path);

  if (myfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::istringstream iss(QTextStream(&myfile).readAll().toStdString());
    std::string errs;
    Json::parseFromStream(json.rbuilder, iss, &JSON, &errs);
    myfile.close();
  } else {
    throw QString("Resource File Missing, Update Resource Folder");
  }

  uint8_t file_size = JSON.size();
  if (JSON[1]["client"])  // file full of client paths
  {
    for (uint8_t i = 0; i < file_size; ++i) {
      std::string client_path = JSON[i]["client"].asString();
      Json::Value json_client;
      QString str = QString::fromStdString(folder_path + client_path);
      QFile myfile(str);
      if (myfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::istringstream iss(QTextStream(&myfile).readAll().toStdString());
        std::string errs;
        Json::parseFromStream(json.rbuilder, iss, &json_client, &errs);
        myfile.close();
      }
      uint8_t custom_client_size = json_client.size();
      for (uint8_t j = 0; j < custom_client_size; ++j) {
        frame_variables_map = CreateFrameVariablesMap(json_client[j], using_custom_order);
      }
    }
  } else  // file with only one client
  {
    for (uint8_t i = 0; i < file_size; ++i) {
      frame_variables_map = CreateFrameVariablesMap(JSON[i], using_custom_order);
    }
  }
  return frame_variables_map;
}

std::map<std::string, FrameVariables *> CreateFrameVariablesMap(const Json::Value &custom_client, bool using_custom_order) {
  std::map<std::string, FrameVariables *> frame_variables_map;
  uint8_t params_size = custom_client["Entries"].size();

  //go through each param in the entry and grab the correct values out
  for (uint8_t j = 0; j < params_size; ++j) {
      //Get the param
    Json::Value param = custom_client["Entries"][j];
    //Find the generic frame vars from the param
    FrameVariables *frame_variables = CreateFrameVariables(param);

    //If you're not using a custom order, use the descriptor as the order (alphabetical)
    //If you are using a custom order, then you need to use the position parameter
    if(!using_custom_order){
        frame_variables_map[param["descriptor"].asString()] = frame_variables;
    }else{
        //Raf set everything up to really really like strings. So, we need to convert the json uint into an ascii value -> string
        char position = param["position"].asUInt();
        std::string position_str(&position, POSITION_BYTE_LEN);
        frame_variables_map[position_str] = frame_variables;
    }
  }

  return frame_variables_map;
}

FrameVariables *CreateFrameVariables(const Json::Value &param) {
  FrameVariables *frame_variables_ptr = new FrameVariables();
  frame_variables_ptr->frame_type_ = param["frame_type"].asUInt();

  if(param.isMember("min_fw_version")){
//      qDebug() << "PCON Version String:" << iv.pcon->GetFirmwareVersionString();
//      iv.pcon->AddToLog("Hi from min fw");
//      qDebug() << "Found a min_fw_version";
//      qDebug() << QString::fromStdString(param["min_fw_version"].asString());
      frame_variables_ptr->min_fw_version_ = QVersionNumber::fromString(QString::fromStdString(param["min_fw_version"].asString()));
//      qDebug() << "Min Version Number: " << frame_variables_ptr->min_fw_version_.toString();
  }else{
    //Give it the smallest possible version if one isn't specified, v0.0.0
    frame_variables_ptr->min_fw_version_ = QVersionNumber(0, 0, 0);
  }

  QVersionNumber max_fw;
  if(param.isMember("max_fw_version")){
//      qDebug() << "Found a max_fw_version";
//      qDebug() << QString::fromStdString(param["max_fw_version"].asString());
      frame_variables_ptr->max_fw_version_ = QVersionNumber::fromString(QString::fromStdString(param["max_fw_version"].asString()));
//      qDebug() << "Max Version Number: " << max_fw.toString();
  }else{
     frame_variables_ptr->max_fw_version_ = QVersionNumber(INT_MAX, INT_MAX, INT_MAX);
  }

  QVersionNumber connected_fw_version = QVersionNumber::fromString(iv.pcon->GetFirmwareVersionString());

  //Only put this in if it is allowed by our firmware version
  if(connected_fw_version >= frame_variables_ptr->min_fw_version_ && connected_fw_version <= frame_variables_ptr->max_fw_version_){
      switch (param["frame_type"].asUInt()) {
        case 1: {
          uint8_t list_size = param["list_name"].size();
          for (uint8_t ii = 0; ii < list_size; ++ii) {
            std::string name = param["list_name"][ii].asString();
            int value = param["list_value"][ii].asInt();
            frame_variables_ptr->combo_frame_.list_names.push_back(name);
            frame_variables_ptr->combo_frame_.list_values.push_back(value);
          }
          frame_variables_ptr->combo_frame_.info = param["info"].asString();
          break;
        }
        case 2: {
          double maximum = NAN;
          double minimum = NAN;
          if (!param["maximum"].isNull()) {
            maximum = param["maximum"].asDouble();
          }
          if (!param["minimum"].isNull()) {
            minimum = param["minimum"].asDouble();
          }
          frame_variables_ptr->spin_frame_.maximum = maximum;
          frame_variables_ptr->spin_frame_.minimum = minimum;
          frame_variables_ptr->spin_frame_.single_step = param["single_step"].asDouble();
          frame_variables_ptr->spin_frame_.decimal = param["decimal"].asDouble();
          frame_variables_ptr->spin_frame_.unit = param["unit"].asString();
          frame_variables_ptr->spin_frame_.nan = param["nan"].asBool();
          frame_variables_ptr->spin_frame_.info = param["info"].asString();
          break;
        }
        case 3: {
          break;
        }
        case 4: {
          double maximum = NAN;
          double minimum = NAN;
          if (!param["maximum"].isNull()) {
            maximum = param["maximum"].asDouble();
          }
          if (!param["minimum"].isNull()) {
            minimum = param["minimum"].asDouble();
          }
          frame_variables_ptr->testing_frame_.maximum = maximum;
          frame_variables_ptr->testing_frame_.minimum = minimum;
          frame_variables_ptr->testing_frame_.single_step = param["single_step"].asDouble();
          frame_variables_ptr->testing_frame_.default_value = param["default_value"].asDouble();
          frame_variables_ptr->testing_frame_.decimal = param["decimal"].asDouble();
          frame_variables_ptr->testing_frame_.unit = param["unit"].asString();
          frame_variables_ptr->testing_frame_.info = param["info"].asString();
          break;
        }
        case 5: {
          frame_variables_ptr->button_frame_.info = param["info"].asString();
          break;
        }
        case 6: {
            double maximum = NAN;
            double minimum = NAN;
            if (!param["maximum"].isNull()) {
              maximum = param["maximum"].asDouble();
            }
            if (!param["minimum"].isNull()) {
              minimum = param["minimum"].asDouble();
            }
            frame_variables_ptr->read_only_frame_.maximum = maximum;
            frame_variables_ptr->read_only_frame_.minimum = minimum;
            frame_variables_ptr->read_only_frame_.single_step = param["single_step"].asDouble();
            frame_variables_ptr->read_only_frame_.decimal = param["decimal"].asDouble();
            frame_variables_ptr->read_only_frame_.unit = param["unit"].asString();
            frame_variables_ptr->read_only_frame_.nan = param["nan"].asBool();
            frame_variables_ptr->read_only_frame_.info = param["info"].asString();
        break;
      }
      }
  }else{
      QString parameter_descriptor = QString::fromStdString(param["descriptor"].asString());
      qDebug() << "Skipping " << parameter_descriptor;
      iv.pcon->AddToLog("Skipped "+parameter_descriptor+" because it is not applicable to firmware version "+iv.pcon->GetFirmwareVersionString());
  }

  return frame_variables_ptr;
}

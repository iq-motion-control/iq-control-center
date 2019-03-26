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


std::map<std::string, FrameVariables*> FrameVariablesFromJson(const std::string &file_name, const std::string &folder_path)
{
  JsonCpp json;
  Json::Value JSON;
  std::map<std::string, FrameVariables*> frame_variables_map;

  QString current_path = QCoreApplication::applicationDirPath();
  QString path = current_path + QString::fromStdString(folder_path + file_name );
  QFile myfile(path);

  if (myfile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    std::istringstream iss(QTextStream(&myfile).readAll().toStdString());
    std::string errs;
    Json::parseFromStream(json.rbuilder, iss, &JSON, &errs);
    myfile.close();
  }
  else
  {
    throw QString("RESOURCE FILE MISSING, UPDATE RESOURCE FOLDER");
  }

  uint8_t file_size = JSON.size();
  if(JSON[1]["client"]) //file full of client paths
  {
    for(uint8_t i = 0; i < file_size; ++i)
    {
      std::string client_path = JSON[i]["client"].asString();
      Json::Value json_client;
      QString str =  QString::fromStdString(folder_path+client_path);
      QFile myfile(str);
      if (myfile.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        std::istringstream iss(QTextStream(&myfile).readAll().toStdString());
        std::string errs;
        Json::parseFromStream(json.rbuilder, iss, &json_client, &errs);
        myfile.close();
      }
      uint8_t custom_client_size = json_client.size();
      for(uint8_t j = 0; j < custom_client_size; ++j)
      {
        frame_variables_map = CreateFrameVariablesMap(json_client[j]);
      }
    }
  }
  else //file with only one client
  {
    for(uint8_t i = 0; i < file_size; ++i)
    {
      frame_variables_map = CreateFrameVariablesMap(JSON[i]);
    }
  }
  return frame_variables_map;
}

std::map<std::string, FrameVariables*> CreateFrameVariablesMap(const Json::Value &custom_client)
{
  std::map<std::string, FrameVariables*> frame_variables_map;
  uint8_t params_size = custom_client["Entries"].size();
  for(uint8_t j = 0; j < params_size; ++j)
  {
    Json::Value param = custom_client["Entries"][j];
    FrameVariables* frame_variables = CreateFrameVariables(param);
    frame_variables_map[param["descriptor"].asString()] = frame_variables;
  }

  return frame_variables_map;
}

FrameVariables* CreateFrameVariables(const Json::Value &param)
{
  FrameVariables* frame_variables_ptr = new FrameVariables();
  frame_variables_ptr->frame_type_ = param["frame_type"].asUInt();
  switch(param["frame_type"].asUInt())
  {
    case 1:
    {
      uint8_t list_size = param["list_name"].size();
      for(uint8_t ii = 0; ii < list_size; ++ii)
      {
        std::string name = param["list_name"][ii].asString();
        uint8_t value = param["list_value"][ii].asUInt();
        frame_variables_ptr->combo_frame_.list_names.push_back(name);
        frame_variables_ptr->combo_frame_.list_values.push_back(value);
      }
      frame_variables_ptr->combo_frame_.info = param["info"].asString();
      break;
    }
    case 2:
    {
      frame_variables_ptr->spin_frame_.maximum = param["maximum"].asDouble();
      frame_variables_ptr->spin_frame_.minimum = param["minimum"].asDouble();
      frame_variables_ptr->spin_frame_.single_step = param["single_step"].asDouble();
      frame_variables_ptr->spin_frame_.decimal = param["decimal"].asDouble();
      frame_variables_ptr->spin_frame_.unit = param["unit"].asString();
      frame_variables_ptr->spin_frame_.nan = param["nan"].asBool();
      frame_variables_ptr->spin_frame_.info = param["info"].asString();
      break;
    }
    case 3:
    {
      break;
    }
    case 4:
    {
      frame_variables_ptr->testing_frame_.maximum = param["maximum"].asDouble();
      frame_variables_ptr->testing_frame_.minimum = param["minimum"].asDouble();
      frame_variables_ptr->testing_frame_.single_step = param["single_step"].asDouble();
      frame_variables_ptr->testing_frame_.decimal = param["decimal"].asDouble();
      frame_variables_ptr->testing_frame_.unit = param["unit"].asString();
      break;
    }
    case 5:
    {
      frame_variables_ptr->button_frame_.info = param["info"].asString();
      break;
    }
  }


  return frame_variables_ptr;
}


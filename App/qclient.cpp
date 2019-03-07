#include "qclient.hpp"

std::unordered_map<std::string, QClient*> QClientsFromJson(const uint8_t &obj_idn, const std::string &file_name, const std::string &folder_path)
{
  JsonCpp json;
  Json::Value JSON;

  std::unordered_map<std::string, QClient*> client_map;

  std::string full_path = folder_path+file_name;

  QString str =  QString::fromStdString(full_path);
  QFile myfile(str);
  if (myfile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    std::istringstream iss(QTextStream(&myfile).readAll().toStdString());
    std::string errs;
    Json::parseFromStream(json.rbuilder, iss, &JSON, &errs);
    myfile.close();
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
        QClient* client_ptr;
        QCreateClient(obj_idn, json_client[j], client_ptr);
        client_map[json_client[j]["descriptor"].asString()] = client_ptr;
      }
    }
  }
  else //file with only one client
  {
    for(uint8_t i = 0; i < file_size; ++i)
    {
      QClient* client_ptr;
      QCreateClient(obj_idn, JSON[i], client_ptr);
      client_map[JSON[i]["descriptor"].asString()] = client_ptr;
    }
  }
  return client_map;
}

void QCreateClient(const uint8_t &obj_idn, const Json::Value &custom_client, QClient* &client_ptr)
{
  std::unordered_map<std::string, QClientEntryAbstract*> client_entry_map;
  uint8_t params_size = custom_client["Entries"].size();
    for(uint8_t j = 0; j < params_size; ++j)
    {
      Json::Value entry = custom_client["Entries"][j];
      QClientEntryAbstract* entry_ptr;
      QCreateClientEntry(obj_idn, custom_client["Entries"][j], entry_ptr);
      client_entry_map[entry["descriptor"].asString()] = entry_ptr;
    }
  client_ptr = new QClient(obj_idn, client_entry_map);
  return;
}

void QCreateClientEntry(const uint8_t &obj_idn, const Json::Value &param, QClientEntryAbstract* &entry_ptr)
{
  char format = param["format"].asCString()[0];
  uint8_t type_idn = param["type_idn"].asUInt();
  uint8_t sub_idn  = param["sub_idn"].asUInt();
  std::string unit = param["unit"].asString();
  uint8_t frame_type = param["frame_type"].asUInt();

  switch(format)
  {
    case 'n':  //empty
    {
      entry_ptr = new QClientEntryVoid(type_idn, obj_idn, sub_idn, format, unit, frame_type);
      break;
    }
    case 'B': // uint8_t
    {
      entry_ptr = new QClientEntry<uint8_t>(type_idn, obj_idn, sub_idn, format, unit, frame_type);
      break;
    }
    case 'b': // int8_t
    {
      entry_ptr = new QClientEntry<int8_t>(type_idn, obj_idn, sub_idn, format, unit, frame_type);
      break;
    }
    case 'f': // float
    {
      entry_ptr = new QClientEntry<float>(type_idn, obj_idn, sub_idn, format, unit, frame_type);
      break;
    }
    case 'H': // uint16_t
    {
      entry_ptr = new QClientEntry<uint16_t>(type_idn, obj_idn, sub_idn, format, unit, frame_type);
      break;
    }
    case 'h': // int16_t
    {
      entry_ptr = new QClientEntry<int16_t>(type_idn, obj_idn, sub_idn, format, unit, frame_type);
      break;
    }
    case 'I': // uint32_t
    {
      entry_ptr = new QClientEntry<uint32_t>(type_idn, obj_idn, sub_idn, format, unit, frame_type);
      break;
    }
    case 'i': // int32_t
    {
      entry_ptr = new QClientEntry<int32_t>(type_idn, obj_idn, sub_idn, format, unit, frame_type);
      break;
    }
  }
  return;
}

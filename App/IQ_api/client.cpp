/*
    Copyright 2018 - 2019 IQ Motion Control   	dskart11@gmail.com

    This file is part of IQ Control Center.

    IQ Control Center is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    IQ Control Center is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "client.hpp"

void Client::UpdateClientObjId(uint8_t new_obj_id){
  //we need to change our object id, as well as that of all of the ClientEntryAbstract objects in our clilent_entry_map_
  obj_idn_ = new_obj_id;

  //Iterate through the map and update obj_idn
  //TODO: obj_idn is public inside of ClientEntryAbstract. We should probably change that
  std::map<std::string, ClientEntryAbstract*>::const_iterator iterator = client_entry_map_.begin();
  while(iterator != client_entry_map_.end()){
    iterator->second->obj_idn_ = new_obj_id;
    iterator++;
  }

}

uint8_t Client::GetClientObjId(){
  return obj_idn_;
}

int Client::Reply(const uint8_t* data, uint8_t len, const std::string& entry_descriptor) {
  ClientEntryAbstract* abstract_entry_ptr = client_entry_map_.at(entry_descriptor);

  char format = abstract_entry_ptr->format_;
  switch (format) {
    case 'n': {
      ClientEntryVoid* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntryVoid*>(abstract_entry_ptr))) return -1;
      entry_ptr->Reply(data, len);
      break;
    }
    case 'B':  // uint8_t
    {
      ClientEntry<uint8_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint8_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->Reply(data, len);
      break;
    }
    case 'b':  // int8_t
    {
      ClientEntry<int8_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int8_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->Reply(data, len);
      break;
    }
    case 'f':  // float
    {
      ClientEntry<float>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<float>*>(abstract_entry_ptr))) return -1;
      entry_ptr->Reply(data, len);
      break;
    }
    case 'H':  // uint16_t
    {
      ClientEntry<uint16_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint16_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->Reply(data, len);
      break;
    }
    case 'h':  // int16_t
    {
      ClientEntry<int16_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int16_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->Reply(data, len);
      break;
    }
    case 'I':  // uint32_t
    {
      ClientEntry<uint32_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint32_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->Reply(data, len);
      break;
    }
    case 'i':  // int32_t
    {
      ClientEntry<int32_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int32_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->Reply(data, len);
      break;
    }
    default: {
      return -1;
    }
  }
  return 1;
}

int Client::Save(CommunicationInterface& com, const std::string& entry_descriptor) {
  ClientEntryAbstract* abstract_entry_ptr = client_entry_map_.at(entry_descriptor);

  char format = abstract_entry_ptr->format_;
  switch (format) {
    case 'n': {
      ClientEntryVoid* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntryVoid*>(abstract_entry_ptr))) return -1;
      entry_ptr->save(com);
      break;
    }
    case 'B':  // uint8_t
    {
      ClientEntry<uint8_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint8_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->save(com);
      break;
    }
    case 'b':  // int8_t
    {
      ClientEntry<int8_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int8_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->save(com);
      break;
    }
    case 'f':  // float
    {
      ClientEntry<float>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<float>*>(abstract_entry_ptr))) return -1;
      entry_ptr->save(com);
      break;
    }
    case 'H':  // uint16_t
    {
      ClientEntry<uint16_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint16_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->save(com);
      break;
    }
    case 'h':  // int16_t
    {
      ClientEntry<int16_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int16_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->save(com);
      break;
    }
    case 'I':  // uint32_t
    {
      ClientEntry<uint32_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint32_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->save(com);
      break;
    }
    case 'i':  // int32_t
    {
      ClientEntry<int32_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int32_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->save(com);
      break;
    }
    default: {
      return -1;
    }
  }
  return 1;
}

int Client::Get(CommunicationInterface& com, const std::string& entry_descriptor) {
  ClientEntryAbstract* abstract_entry_ptr = client_entry_map_.at(entry_descriptor);

  char format = abstract_entry_ptr->format_;
  switch (format) {
    case 'n': {
      ClientEntryVoid* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntryVoid*>(abstract_entry_ptr))) return -1;
      entry_ptr->get(com);
      break;
    }
    case 'B':  // uint8_t
    {
      ClientEntry<uint8_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint8_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->get(com);
      break;
    }
    case 'b':  // int8_t
    {
      ClientEntry<int8_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int8_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->get(com);
      break;
    }
    case 'f':  // float
    {
      ClientEntry<float>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<float>*>(abstract_entry_ptr))) return -1;
      entry_ptr->get(com);
      break;
    }
    case 'H':  // uint16_t
    {
      ClientEntry<uint16_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint16_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->get(com);
      break;
    }
    case 'h':  // int16_t
    {
      ClientEntry<int16_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int16_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->get(com);
      break;
    }
    case 'I':  // uint32_t
    {
      ClientEntry<uint32_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint32_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->get(com);
      break;
    }
    case 'i':  // int32_t
    {
      ClientEntry<int32_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int32_t>*>(abstract_entry_ptr))) return -1;
      entry_ptr->get(com);
      break;
    }
    default: {
      return -1;
    }
  }
  return 1;
}

int Client::IsFresh(const std::string& entry_descriptor, bool& is_fresh) {
  ClientEntryAbstract* abstract_entry_ptr = client_entry_map_.at(entry_descriptor);

  char format = abstract_entry_ptr->format_;
  switch (format) {
    case 'n': {
      ClientEntryVoid* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntryVoid*>(abstract_entry_ptr))) return -1;
      is_fresh = entry_ptr->IsFresh();
      break;
    }
    case 'B':  // uint8_t
    {
      ClientEntry<uint8_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint8_t>*>(abstract_entry_ptr))) return -1;
      is_fresh = entry_ptr->IsFresh();
      break;
    }
    case 'b':  // int8_t
    {
      ClientEntry<int8_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int8_t>*>(abstract_entry_ptr))) return -1;
      is_fresh = entry_ptr->IsFresh();
      break;
    }
    case 'f':  // float
    {
      ClientEntry<float>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<float>*>(abstract_entry_ptr))) return -1;
      is_fresh = entry_ptr->IsFresh();
      break;
    }
    case 'H':  // uint16_t
    {
      ClientEntry<uint16_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint16_t>*>(abstract_entry_ptr))) return -1;
      is_fresh = entry_ptr->IsFresh();
      break;
    }
    case 'h':  // int16_t
    {
      ClientEntry<int16_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int16_t>*>(abstract_entry_ptr))) return -1;
      is_fresh = entry_ptr->IsFresh();
      break;
    }
    case 'I':  // uint32_t
    {
      ClientEntry<uint32_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<uint32_t>*>(abstract_entry_ptr))) return -1;
      is_fresh = entry_ptr->IsFresh();
      break;
    }
    case 'i':  // int32_t
    {
      ClientEntry<int32_t>* entry_ptr = nullptr;
      if (!(entry_ptr = dynamic_cast<ClientEntry<int32_t>*>(abstract_entry_ptr))) return -1;
      is_fresh = entry_ptr->IsFresh();
      break;
    }
    default: {
      return -1;
    }
  }
  return 1;
}

int Client::Set(CommunicationInterface& com, const std::string& entry_descriptor) {
  ClientEntryAbstract* abstract_entry_ptr = client_entry_map_.at(entry_descriptor);
  ClientEntryVoid* entry_ptr = nullptr;
  if (!(entry_ptr = dynamic_cast<ClientEntryVoid*>(abstract_entry_ptr))) return -1;
  entry_ptr->set(com);
  return 1;
}

void Client::ReadMsg(CommunicationInterface& com, uint8_t* rx_data, uint8_t rx_length) {
  ParseMsg(rx_data, rx_length, client_entry_map_);
}

Client::~Client() {
  for (const auto& n : client_entry_map_) {
    delete n.second;
  }
}

int8_t ParseMsg(uint8_t* rx_data, uint8_t rx_length,
                const std::map<std::string, ClientEntryAbstract*>& client_entry_map) {
  uint8_t type_idn = rx_data[0];
  uint8_t sub_idn = rx_data[1];
  uint8_t obj_idn = rx_data[2] >> 2;                          // high 8 bits are obj_idn
  Access dir = static_cast<Access>(rx_data[2] & 0b00000011);  // low two bits

  // if we have a reply (we only parse replies here)
  if (dir == kReply) {
    for (const auto& n : client_entry_map) {
      // if sub_idn exists in the client
      if (sub_idn == n.second->sub_idn_) {
        // if the type and obj identifiers match
        if (n.second->type_idn_ == type_idn && n.second->obj_idn_ == obj_idn) {
          // ... then we have a valid message
          n.second->Reply(&rx_data[3], rx_length - 3);
          return 1;  // I parsed something
        }
      }
    }
  }
  return 0;
}

std::map<std::string, Client*> ClientsFromJson(uint8_t obj_idn, const std::string& file_name,
                                               const std::string& folder_path, bool * using_custom_order,
                                               std::map<std::string, std::string> * client_descriptor_map) {
  JsonCpp json;
  Json::Value JSON;

  std::map<std::string, Client*> client_map;

  QString file_path = QString::fromStdString(folder_path + file_name);
  QFile myfile(file_path);
  if (myfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::istringstream iss(QTextStream(&myfile).readAll().toStdString());
    std::string errs;
    Json::parseFromStream(json.rbuilder, iss, &JSON, &errs);
    myfile.close();
  } else {
    throw QString("RESOURCE FILE MISSING, UPDATE RESOURCE FOLDER");
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
        Client* client_ptr;
        CreateClient(obj_idn, json_client[j], client_ptr, using_custom_order, client_descriptor_map);
        client_map[json_client[j]["descriptor"].asString()] = client_ptr;
      }
    }
  } else  // file with only one client
  {
    for (uint8_t i = 0; i < file_size; ++i) {
      Client* client_ptr;
      CreateClient(obj_idn, JSON[i], client_ptr, using_custom_order, client_descriptor_map);
      client_map[JSON[i]["descriptor"].asString()] = client_ptr;
    }
  }
  return client_map;
}

void CreateClient(uint8_t obj_idn, const Json::Value& custom_client, Client*& client_ptr, bool * using_custom_order, std::map<std::string, std::string> * client_descriptor_map) {
  std::map<std::string, ClientEntryAbstract*> client_entry_map;
  std::map<std::string, std::string> local_client_descriptor_map;

  uint8_t params_size = custom_client["Entries"].size();

  //Read whether or not this tab has a custom order or if we should just use alphabetical
  bool custom_order = custom_client["custom_order"].asBool();

  if(using_custom_order != nullptr){
    *using_custom_order = custom_order;
  }

  for (uint8_t j = 0; j < params_size; ++j) {
    Json::Value entry = custom_client["Entries"][j];
    std::string entry_descriptor = entry["descriptor"].asString(); //Every entry regardless of ordering gets their descriptor grabbed here

    ClientEntryAbstract* entry_ptr;
    CreateClientEntry(obj_idn, custom_client["Entries"][j], entry_ptr);

    if(!custom_order){
        client_entry_map[entry["descriptor"].asString()] = entry_ptr;
    }else{
        char position = entry["position"].asUInt();
        std::string position_str(&position, 1);

        //We'll need a map to store the entries we want in the order we want
        //In order to get the correct name with the correct entry, we'll need another map to store the descriptor
        client_entry_map[position_str] = entry_ptr;
        local_client_descriptor_map[position_str] = entry_descriptor;
    }
  }

  if(client_descriptor_map != nullptr){
    client_descriptor_map->swap(local_client_descriptor_map); //Bring the descriptor data out of this fxn
  }

  client_ptr = new Client(obj_idn, client_entry_map);
  return;
}

void CreateClientEntry(uint8_t obj_idn, const Json::Value& param, ClientEntryAbstract*& entry_ptr) {
  char format = param["format"].asCString()[0];
  uint8_t type_idn = param["type_idn"].asUInt();
  uint8_t sub_idn = param["sub_idn"].asUInt();
  std::string unit = param["unit"].asString();

  switch (format) {
    case 'n':  // empty
    {
      entry_ptr = new ClientEntryVoid(type_idn, obj_idn, sub_idn, format, unit);
      break;
    }
    case 'B':  // uint8_t
    {
      entry_ptr = new ClientEntry<uint8_t>(type_idn, obj_idn, sub_idn, format, unit);
      break;
    }
    case 'b':  // int8_t
    {
      entry_ptr = new ClientEntry<int8_t>(type_idn, obj_idn, sub_idn, format, unit);
      break;
    }
    case 'f':  // float
    {
      entry_ptr = new ClientEntry<float>(type_idn, obj_idn, sub_idn, format, unit);
      break;
    }
    case 'H':  // uint16_t
    {
      entry_ptr = new ClientEntry<uint16_t>(type_idn, obj_idn, sub_idn, format, unit);
      break;
    }
    case 'h':  // int16_t
    {
      entry_ptr = new ClientEntry<int16_t>(type_idn, obj_idn, sub_idn, format, unit);
      break;
    }
    case 'I':  // uint32_t
    {
      entry_ptr = new ClientEntry<uint32_t>(type_idn, obj_idn, sub_idn, format, unit);
      break;
    }
    case 'i':  // int32_t
    {
      entry_ptr = new ClientEntry<int32_t>(type_idn, obj_idn, sub_idn, format, unit);
      break;
    }
  }
  return;
}

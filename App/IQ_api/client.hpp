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

/*
 * client_helpers.hpp
 *
 *  Created on: August 23, 2018
 *      Author: Raphael Van Hoffelen
 *
 * Create Clients from json
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <fstream>
#include <string>
#include <vector>
//#include <map>
#include <map>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTextStream>
#include "client_communication.hpp"
#include "json_cpp.hpp"

class Client {
 public:
  Client(uint8_t obj_idn, std::map<std::string, ClientEntryAbstract*> client_entry_map)
      : obj_idn_(obj_idn), client_entry_map_(client_entry_map) {}

  ~Client();

  const uint8_t obj_idn_;
  const std::map<std::string, ClientEntryAbstract*> client_entry_map_;

  void ReadMsg(CommunicationInterface& com, uint8_t* rx_data, uint8_t rx_length);

  int Reply(const uint8_t* data, uint8_t len, const std::string& entry_descriptor);

  int Save(CommunicationInterface& com, const std::string& entry_descriptor);

  int Get(CommunicationInterface& com, const std::string& entry_descriptor);

  int Set(CommunicationInterface& com, const std::string& entry_descriptor);

  int IsFresh(const std::string& entry_descriptor, bool& is_fresh);

  template <typename T>
  int Set(CommunicationInterface& com, const std::string& entry_descriptor, T value) {
    ClientEntryAbstract* abstract_entry_ptr = client_entry_map_.at(entry_descriptor);

    char format = abstract_entry_ptr->format_;
    switch (format) {
      case 'B':  // uint8_t
      {
        ClientEntry<uint8_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<uint8_t>*>(abstract_entry_ptr))) return -1;
        entry_ptr->set(com, value);
        break;
      }
      case 'b':  // int8_t
      {
        ClientEntry<int8_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<int8_t>*>(abstract_entry_ptr))) return -1;
        entry_ptr->set(com, value);
        break;
      }
      case 'f':  // float
      {
        ClientEntry<float>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<float>*>(abstract_entry_ptr))) return -1;
        entry_ptr->set(com, value);
        break;
      }
      case 'H':  // uint16_t
      {
        ClientEntry<uint16_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<uint16_t>*>(abstract_entry_ptr))) return -1;
        entry_ptr->set(com, value);
        break;
      }
      case 'h':  // int16_t
      {
        ClientEntry<int16_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<int16_t>*>(abstract_entry_ptr))) return -1;
        entry_ptr->set(com, value);
        break;
      }
      case 'I':  // uint32_t
      {
        ClientEntry<uint32_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<uint32_t>*>(abstract_entry_ptr))) return -1;
        entry_ptr->set(com, value);
        break;
      }
      case 'i':  // int32_t
      {
        ClientEntry<int32_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<int32_t>*>(abstract_entry_ptr))) return -1;
        entry_ptr->set(com, value);
        break;
      }
      default: {
        return -1;
      }
    }
    return 1;
  }

  template <typename T>
  int GetReply(const std::string& entry_descriptor, T& value) {
    ClientEntryAbstract* abstract_entry_ptr = client_entry_map_.at(entry_descriptor);
    char format = abstract_entry_ptr->format_;
    switch (format) {
      case 'B':  // uint8_t
      {
        ClientEntry<uint8_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<uint8_t>*>(abstract_entry_ptr))) return -1;
        value = entry_ptr->get_reply();
        break;
      }
      case 'b':  // int8_t
      {
        ClientEntry<int8_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<int8_t>*>(abstract_entry_ptr))) return -1;
        value = entry_ptr->get_reply();
        break;
      }
      case 'f':  // float
      {
        ClientEntry<float>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<float>*>(abstract_entry_ptr))) return -1;
        value = entry_ptr->get_reply();
        break;
      }
      case 'H':  // uint16_t
      {
        ClientEntry<uint16_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<uint16_t>*>(abstract_entry_ptr))) return -1;
        value = entry_ptr->get_reply();
        break;
      }
      case 'h':  // int16_t
      {
        ClientEntry<int16_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<int16_t>*>(abstract_entry_ptr))) return -1;
        value = entry_ptr->get_reply();
        break;
      }
      case 'I':  // uint32_t
      {
        ClientEntry<uint32_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<uint32_t>*>(abstract_entry_ptr))) return -1;
        value = entry_ptr->get_reply();
        break;
      }
      case 'i':  // int32_t
      {
        ClientEntry<int32_t>* entry_ptr = nullptr;
        if (!(entry_ptr = dynamic_cast<ClientEntry<int32_t>*>(abstract_entry_ptr))) return -1;
        value = entry_ptr->get_reply();
        break;
      }
      default: {
        return -1;
      }
    }
    return 1;
  }
};

int8_t ParseMsg(uint8_t* rx_data, uint8_t rx_length,
                const std::map<std::string, ClientEntryAbstract*>& client_entry_map);
std::map<std::string, Client*> ClientsFromJson(const uint8_t& obj_idn, const std::string& file_name,
                                               const std::string& folder_path, bool * using_custom_order,
                                               std::map<std::string, std::string> * client_descriptor_map);

void CreateClientEntry(uint8_t obj_idn, const Json::Value& param,
                       ClientEntryAbstract*& abstract_entry_ptr);

void CreateClient(const uint8_t& obj_idn, const Json::Value& custom_client, Client*& client_ptr, bool * using_custom_order, std::map<std::string, std::string> * client_descriptor_map);

#endif

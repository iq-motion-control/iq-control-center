#ifndef QCLIENT_HPP
#define QCLIENT_HPP

#include <memory>

#include "IQ_api/client.hpp"
#include "qclient_communication.hpp"

class QClient
{
  public:
    QClient(uint8_t obj_idn, std::unordered_map<std::string, QClientEntryAbstract*> qclient_entry_map):
      qclient_entry_map_(qclient_entry_map)
    {
      std::unordered_map<std::string, ClientEntryAbstract*> client_entry_map;
      for(std::pair<std::string, QClientEntryAbstract*> qclient_entry: qclient_entry_map_)
      {
        client_entry_map[qclient_entry.first] = qclient_entry.second;
      }

      client_ = new Client(obj_idn, client_entry_map);
    }

    ~QClient(){delete client_;}

    const std::unordered_map<std::string, QClientEntryAbstract*> qclient_entry_map_;
    Client *client_;
};

std::unordered_map<std::string, QClient*> QClientsFromJson(const uint8_t &obj_idn, const std::string &file_name, const std::string &folder_path);
void QCreateClientEntry(const uint8_t &obj_idn, const Json::Value &param, QClientEntryAbstract* &abstract_entry_ptr);
void QCreateClient(const uint8_t &obj_idn, const Json::Value &custom_client, QClient* &client_ptr);
#endif // QCLIENT_HPP

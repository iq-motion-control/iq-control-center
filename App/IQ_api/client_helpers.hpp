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
 * Helper functions that make getting & setting easier
 * also handles deleting all clients in map
 */

#ifndef CLIENT_HELPERS_HPP
#define CLIENT_HELPERS_HPP

#include <iostream>
#include <math.h>
#include <map>
#include <time.h>
#include <ratio>
#include <chrono>

#include "communication_interface.h"
#include "client_communication.hpp"
#include "client.hpp"

void DeleteClientMap(std::map<std::string, Client*> client_map);

void ReadMessage(CommunicationInterface &com, Client* &client_ptr);

bool GetEntry(CommunicationInterface &com, Client* &client_ptr, const std::string &entry_descriptor, uint32_t retries, float timeout);

template <typename T>
bool GetEntryReply(CommunicationInterface &com, Client* &client_ptr, const std::string &entry_descriptor, uint32_t retries, float timeout, T &reply)
{
  if(GetEntry(com, client_ptr, entry_descriptor, retries, timeout))
  {
    int err = client_ptr->GetReply(entry_descriptor, reply);
    if(err <=0)
      return false;
    return true;
  }
  return false;
}

template <typename T>
T GetEntryReplyOrDie(CommunicationInterface& com, Client* &client_ptr,  const std::string &entry_descriptor, uint32_t retries, float timeout)
{
  T reply;
  if(!GetEntryReply(com,client_ptr,entry_descriptor,retries,timeout,reply))
    throw std::string("GetEntryReply died") ;
  return reply;
}

template <typename T>
bool SetVerifyEntry(CommunicationInterface &com, Client* &client_ptr, const std::string &entry_descriptor, uint32_t retries, float timeout, T value)
{
  client_ptr->Set(com, entry_descriptor, value);
  com.SendNow();

  ClientEntryAbstract* abstract_entry_ptr = client_ptr->client_entry_map_.at(entry_descriptor);
  char format = abstract_entry_ptr->format_;
  float ratio = 0.1;
  switch(format)
  {
    case 'B': // uint8_t
    {
      uint8_t check_value;
      if((!GetEntryReply(com,client_ptr,entry_descriptor,retries,timeout,check_value)))
        return false;
      uint8_t a = (uint8_t)value;
      uint8_t b = check_value;
      if(((a + fabs(a * ratio)) >= b) && ((a - fabs(a * ratio)) <= b))
        break;
      return false;
    }
    case 'b': // int8_t
    {
      int8_t check_value;
      if((!GetEntryReply(com,client_ptr,entry_descriptor,retries,timeout,check_value)))
        return false;
      int8_t a = (int8_t)value;
      int8_t b = check_value;
      if(((a + fabs(a * ratio)) >= b) && ((a - fabs(a * ratio)) <= b))
        break;
      return false;
    }
    case 'f': // float
    {
      float check_value;
      if((!GetEntryReply(com,client_ptr,entry_descriptor,retries,timeout,check_value)))
        return false;
      float a = (float)value;
      float b = check_value;
      if(((a + fabs(a * ratio)) >= b) && ((a - fabs(a * ratio)) <= b))
        break;
      return false;
    }
    case 'H': // uint16_t
    {
      uint16_t check_value;
      if((!GetEntryReply(com,client_ptr,entry_descriptor,retries,timeout,check_value)))
        return false;
      uint16_t a = (uint16_t)value;
      uint16_t b = check_value;
      if(((a + fabs(a * ratio)) >= b) && ((a - fabs(a * ratio)) <= b))
        break;
      return false;
    }
    case 'h': // int16_t
    {
      int16_t check_value;
      if((!GetEntryReply(com,client_ptr,entry_descriptor,retries,timeout,check_value)))
        return false;
      int16_t a = (int16_t)value;
      int16_t b = check_value;
      if(((a + fabs(a * ratio)) >= b) && ((a - fabs(a * ratio)) <= b))
        break;
      return false;
    }
    case 'I': // uint32_t
    {
      uint32_t check_value;
      if((!GetEntryReply(com,client_ptr,entry_descriptor,retries,timeout,check_value)))
        return false;
      uint32_t a = (uint32_t)value;
      uint32_t b = check_value;
      if(((a + fabs(a * ratio)) >= b) && ((a - fabs(a * ratio)) <= b))
        break;
      return false;
    }
    case 'i': // int32_t
    {
      int32_t check_value;
      if((!GetEntryReply(com,client_ptr,entry_descriptor,retries,timeout,check_value)))
        return false;
      int32_t a = (int32_t)value;
      int32_t b = check_value;
      if(((a + fabs(a * ratio)) >= b) && ((a - fabs(a * ratio)) <= b))
        break;
      return false;
    }
    default :
    {
      return false;
    }
  }
  return true;
}

template <typename T>
bool SetVerifyEntrySave(CommunicationInterface &com, Client* &client_ptr, const std::string &entry_descriptor, uint32_t retries, float timeout, T& value)
{
  if(SetVerifyEntry(com, client_ptr, entry_descriptor, retries, timeout, value))
  {
    client_ptr->Save(com, entry_descriptor);
    com.SendNow();
    return true;
  }
  return 0;
}

template <typename T>
void SetVerifyEntrySaveEntryOrDie(CommunicationInterface &com, Client* &client_ptr, const std::string &entry_descriptor, uint32_t retries, float timeout, T& value)
{
  if(!SetVerifyEntrySaveEntry(com,client_ptr,entry_descriptor,retries,timeout,value))
    throw std::string("SetVerifyEntrySave died");
  return;
}

#endif


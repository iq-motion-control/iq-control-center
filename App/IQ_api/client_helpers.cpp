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

#include "client_helpers.hpp"

void DeleteClientMap(std::map<std::string, Client*> client_map)
{
  for( const auto& n : client_map)
  {
    delete n.second;
  }
}

void ReadMessage(CommunicationInterface &com, Client* &client_ptr)
{
  uint8_t *rx_data;
  uint8_t rx_length;

  com.GetBytes();
  while(com.PeekPacket(&rx_data,&rx_length))
  {
    client_ptr->ReadMsg(com,rx_data,rx_length);
    com.DropPacket();
  }
}


bool GetEntry(CommunicationInterface &com, Client* &client_ptr, const std::string &entry_descriptor, uint32_t retries, float timeout)
{
  com.Flush();
  using namespace std::chrono;

  float time_wait;
  int err = 0;
  bool result = false;
  for(uint8_t i = 0; i < retries; i++)
  {
    err = client_ptr->Get(com, entry_descriptor);
    if(err <= 0)
    	return false;
    com.SendNow();
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    time_wait = 0;
    while(time_wait < timeout && result == false)
    {
      ReadMessage(com,client_ptr);
      err = client_ptr->IsFresh(entry_descriptor, result);
      if(err <= 0)
        return false;
      high_resolution_clock::time_point t2 = high_resolution_clock::now();
      duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
      time_wait = time_span.count();
    }

    if(result)
      break;
  }
  return result;
}

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

#include "firmware.h"

Firmware::Firmware()
{
  sys_map_ = ClientsFromJson(0, "system_control_client.json",  clients_folder_path_);
}

void Firmware::BootModeSelected()
{
  if(iv.pcon->GetConnectionState() == 1)
  {
    try
    {
      QSerialInterface *ser = iv.pcon->GetQSerialInterface();
      if(!sys_map_["system_control_client"]->Set(*ser, std::string("reboot_boot_loader")))
        throw QString("COULDN'T REBOOT: please reconnect or try again");
      ser->SendNow();
      QString success_message = "BOOT MODE SET SUCCESSFULLY";
      iv.label_message->setText(success_message);

//      delete and unconnects port
      delete ser->ser_port_;
      iv.pcon->SetPortConnection(0);
      emit iv.pcon->LostConnection();

    }
    catch(const QString &e)
    {
      iv.label_message->setText(e);
    }
  }
  else
  {
    QString error_message = "NO MOTOR CONNECTED, PLEASE CONNECT MOTOR";
    iv.label_message->setText(error_message);
  }
}

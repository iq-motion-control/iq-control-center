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

#include "testing.h"

Testing::Testing()
{
}

void Testing::CreateClient()
{
  mot_map_ = ClientsFromJson(iv.pcon->GetObjId(), "safe_brushless_drive_client.json",  ":/IQ_api/clients/");
}

void Testing::Coast()
{
  if(iv.pcon->GetConnectionState() == 1)
  {
    try
    {
      mot_map_["safe_brushless_drive_client"]->Set(*iv.pcon->GetQSerialInterface(), "drive_coast");
      iv.pcon->GetQSerialInterface()->SendNow();
      QString success_message = "SET MOTOR TO COAST";
      iv.label_message->setText(success_message);
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

void Testing::Brake()
{
  if(iv.pcon->GetConnectionState() == 1)
  {
    try
    {
      mot_map_["safe_brushless_drive_client"]->Set( *iv.pcon->GetQSerialInterface(), "drive_brake");
      iv.pcon->GetQSerialInterface()->SendNow();
      QString success_message = "SET MOTOR TO BRAKE";
      iv.label_message->setText(success_message);
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

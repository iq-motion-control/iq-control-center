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

#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <QObject>

#include "main.h"

#include "IQ_api/client.hpp"

class Firmware : public QObject
{
    Q_OBJECT
  public:
    Firmware();

  private:
    std::string clients_folder_path_ = ":/IQ_api/clients/";
    std::map<std::string, Client*> sys_map_;

  signals:

  public slots:
    void BootModeSelected();
};

#endif // FIRMWARE_H

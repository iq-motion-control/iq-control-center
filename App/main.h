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

#ifndef MAIN_HPP
#define MAIN_HPP

#include "qserial_interface.h"
#include <QLabel>
#include "port_connection.h"

struct InterfaceVariables
{
  PortConnection *pcon = nullptr;
  QLabel *label_message = nullptr;
};

extern InterfaceVariables iv;

#endif // MAIN_HPP

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

#ifndef TESTING_H
#define TESTING_H

#include <QObject>
#include "IQ_api/client.hpp"
#include "main.h"

class Testing : public QObject
{
    Q_OBJECT
  public:
    Testing();

    std::map<std::string, Client*> mot_map_;

  signals:

  public slots:
    void CreateClient();
    void Coast();
    void Brake();
};

#endif // TESTING_H

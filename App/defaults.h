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

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QComboBox>
#include <QCoreApplication>

#include <string>
#include <unordered_map>
#include <map>
#include "IQ_api/json_cpp.hpp"


class Defaults : public QObject
{
    Q_OBJECT
  public:
    Defaults(QComboBox* comb, std::string folder_path);

  signals:
   void SaveDefaultValues(Json::Value);

  public slots:
   void LoadDefaultValues();

   void DefaultComboBoxIndexChanged(int index);

  private:
   QComboBox* comb_;
   std::unordered_map<std::string, std::string> default_file_map_;
   std::string folder_path_;
   Json::Value default_values_;
   std::vector<Json::Value> defaults_;


};

Json::Value DefaultValueFromJson(const QString &file_name, const QString &folder_path);

#endif // DEFAULTS_H

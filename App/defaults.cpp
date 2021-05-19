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

#include "defaults.h"

Defaults::Defaults(QComboBox* comb, std::string folder_path) :
  comb_(comb),
  folder_path_(folder_path)
{
  QString current_path = QCoreApplication::applicationDirPath();
  QString path = current_path + QString::fromStdString(folder_path);

  QDir dir(path);

  QStringList files = dir.entryList(QDir::Files);

  if (files.size() != 0)
  {
    comb_->addItems(files);
    comb_->show();
    for ( const QString& file : files  )
    {
      defaults_.push_back(DefaultValueFromJson(file, path));
    }

    default_values_ = defaults_[0];
  }
}


void Defaults::LoadDefaultValues()
{
  emit SaveDefaultValues(default_values_);
}



void Defaults::DefaultComboBoxIndexChanged(int index)
{
  default_values_ = defaults_[index];
}

Json::Value DefaultValueFromJson(const QString &file_name, const QString &folder_path)
{
  JsonCpp json;
  Json::Value JSON;

  QString full_path = folder_path+file_name;

  QFile myfile(full_path);

  if (myfile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    std::istringstream iss(QTextStream(&myfile).readAll().toStdString());
    std::string errs;
    Json::parseFromStream(json.rbuilder, iss, &JSON, &errs);
    myfile.close();
  }
  else
  {
    throw QString("Resrouce File Missing, Update Resource Folder");
  }
  return JSON;
}

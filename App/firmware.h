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

#include <chrono>

#include <QFileDialog>
#include <QJsonDocument>
#include <QObject>
#include <QProgressBar>
#include <QStandardPaths>
#include <QTime>
#include <QVector>

#include "IQ_api/client.hpp"
#include "Schmi/binary_file_std.hpp"
#include "Schmi/flash_loader.hpp"
#include "Schmi/loading_bar_std.hpp"
#include "Schmi/qerror_handler.hpp"
#include "Schmi/qserial.h"

#include "flash_loading_bar.h"

#include "main.h"

class Firmware : public QObject {
  Q_OBJECT
 public:
  Firmware(QProgressBar *flash_progress_bar, QPushButton *firmware_binary_button);

 private:
  QString firmware_folder_dir_name_ = "";
  std::string clients_folder_path_ = ":/IQ_api/clients/";
  std::map<std::string, Client *> sys_map_;
  QProgressBar *flash_progress_bar_;
  QPushButton *firmware_binary_button_;
  QString firmware_bin_path_;

  bool BootMode();

 signals:

 public slots:
  void FlashClicked();
  void SelectFirmwareClicked();
};

#endif  // FIRMWARE_H

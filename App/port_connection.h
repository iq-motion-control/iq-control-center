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

#ifndef PORT_CONNECTION_HPP
#define PORT_CONNECTION_HPP

#include <QMainWindow>
#include <QObject>
#include <QMessageBox>
#include <QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "qserial_interface.h"
#include "ui_mainwindow.h"

#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"

#include <QMetaEnum>
#include <QMetaObject>


#define MAJOR_VERSION_MASK 0x000fc000
#define MINOR_VERSION_MASK 0x00003f80
#define PATCH_VERSION_MASK 0x7f

#define MAJOR_VERSION_SHIFT 14
#define MINOR_VERSION_SHIFT 7

class PortConnection : public QObject {
  Q_OBJECT
 public:
  enum ExtraBaudRate{
    Baud921600 = 921600
  };

  PortConnection(Ui::MainWindow *user_in);

  ~PortConnection() {}

  bool CheckIfInBootLoader();

  void SetPortConnection(bool state);
  void FindBaudrates();

  bool GetConnectionState() { return connection_state_; }
  void SetConnectionState(const bool &setter) { connection_state_ = setter; }

  QSerialInterface *GetQSerialInterface() { return ser_; }
  void SetQSerialInterface(QSerialInterface *setter) { ser_ = setter; }

  uint8_t GetObjId() { return obj_id_; }
  void SetObjId(const uint8_t &setter) { obj_id_ = setter; }

  int GetFirmwareStyle() { return firmware_style_; }
  void SetFirmwareStyle(const int &setter) { firmware_style_ = setter; }

  int GetHardwareType() { return hardware_type_; }
  void SetHardwareType(const int &setter) { hardware_type_ = setter; }

  QString GetSelectedPortName() { return selected_port_name_; }
  QString GetRecoveryPortName() { return recovery_port_name_; }

 public slots:

  void ConnectMotor();

  void TimerTimeout();

  void PortError(QSerialPort::SerialPortError error);

  void FindPorts();

  void PortComboBoxIndexChanged(int index);

  void BaudrateComboBoxIndexChanged(int index);

 signals:

  void ObjIdFound();

  void FindSavedValues();

  void TypeStyleFound(int, int, int);

  void LostConnection();

 private:
  Ui::MainWindow *ui_;
  QSerialInterface *ser_;

  std::string clients_folder_path_ = ":/IQ_api/clients/";
  std::map<std::string, Client *> sys_map_;

  std::vector<QString> ports_names_;
  QString selected_port_name_;
  QString recovery_port_name_;
  qint32 selected_baudrate_;

  QMovie *loader_movie_;

  bool connection_state_;

  uint8_t obj_id_;
  int firmware_style_;
  int hardware_type_;
};

#endif  // CONNECTION_HPP

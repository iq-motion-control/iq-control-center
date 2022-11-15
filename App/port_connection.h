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

#define BOOT_MAJOR_MASK 0xf800
#define BOOT_MINOR_MASK 0x07C0
#define BOOT_PATCH_MASK 0x003f

#define BOOT_MAJOR_SHIFT 11
#define BOOT_MINOR_SHIFT 6

#define UPGRADE_MAJOR_MASK 0xf800
#define UPGRADE_MINOR_MASK 0x07C0
#define UPGRADE_PATCH_MASK 0x003f

#define UPGRADE_MAJOR_SHIFT 11
#define UPGRADE_MINOR_SHIFT 6

class PortConnection : public QObject {
  Q_OBJECT
 public:
  enum ExtraBaudRate{
    Baud921600 = 921600
  };

  PortConnection(Ui::MainWindow *user_in);

  ~PortConnection() {}

  /**
   * @brief CheckIfInBootLoader Checks if the motor is currently in the ST Bootloader
   * @return true if it is. False if not
   */
  bool CheckIfInBootLoader();

  /**
   * @brief SetPortConnection Sets up the connection UI
   * @param state true if already connected
   */
  void SetPortConnection(bool state);

  /**
   * @brief FindBaudrates Adds different baud rates to the dropdown UI
   */
  void FindBaudrates();

  /**
   * @brief GetConnectionState Returns the current state of connection
   * @return true if connected
   */
  bool GetConnectionState() { return connection_state_; }

  /**
   * @brief SetConnectionState Sets the current connection state
   * @param setter
   */
  void SetConnectionState(const bool &setter) { connection_state_ = setter; }

  /**
   * @brief GetQSerialInterface Returns the serial interface being used
   * @return the curent serial interface
   */
  QSerialInterface *GetQSerialInterface() { return ser_; }

  /**
   * @brief SetQSerialInterface Sets the serial interface
   * @param setter
   */
  void SetQSerialInterface(QSerialInterface *setter) { ser_ = setter; }

  /**
   * @brief GetObjId
   * @return
   */
  uint8_t GetObjId() { return obj_id_; }
  /**
   * @brief SetObjId
   * @param setter
   */
  void SetObjId(const uint8_t &setter) { obj_id_ = setter; }

  /**
   * @brief GetFirmwareStyle
   * @return
   */
  int GetFirmwareStyle() { return firmware_style_; }
  /**
   * @brief SetFirmwareStyle
   * @param setter
   */
  void SetFirmwareStyle(const int &setter) { firmware_style_ = setter; }

  /**
   * @brief GetHardwareType
   * @return
   */
  int GetHardwareType() { return hardware_type_; }
  /**
   * @brief SetHardwareType
   * @param setter
   */
  void SetHardwareType(const int &setter) { hardware_type_ = setter; }

  /**
   * @brief GetElectronicsType
   * @return
   */
  int GetElectronicsType() { return electronics_type_; }

  /**
   * @brief SaveNewBootloaderVersion Sends a command to the motor to save the bootloader version being flashed
   * @param newVersion the raw value of the bootloader version
   */
  void SaveNewBootloaderVersion(uint16_t newVersion);

  /**
   * @brief SaveNewUpgraderVersion Sends a command to the motor to save the upgrader version being flashed
   * @param newVersion the raw value of the upgrader version
   */
  void SaveNewUpgraderVersion(uint16_t newVersion);

  /**
   * @brief GetAppsPresent Returns 3 bits stored at the bottom of this byte that indicate which sections are on the motor
   * @return
   */
  uint8_t GetAppsPresent() { return applications_present_on_motor_; }

  /**
   * @brief GetSelectedPortName Returns a string of the serial port being used
   * @return
   */
  QString GetSelectedPortName() { return selected_port_name_; }

  /**
   * @brief GetRecoveryPortName Returns a string of the serial port to use during recovery
   * @return
   */
  QString GetRecoveryPortName() { return recovery_port_name_; }

  /**
   * @brief ResetToTopPage Resets the control center back to its default state
   */
  void ResetToTopPage();

  /**
   * @brief GetCurrentTab returns the current tab being displayed
   * @return
   */
  int GetCurrentTab();

  /**
   * @brief GetMainWindowAccess returns a pointer to the user interface for other modules to use
   * @return
   */
  Ui::MainWindow* GetMainWindowAccess() { return ui_;}

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

  std::string clients_folder_path_ = ":/IQ_api/clients/";
  std::map<std::string, Client *> sys_map_;

  std::vector<QString> ports_names_;
  QString selected_port_name_;
  QString recovery_port_name_;
  qint32 selected_baudrate_;

  QMovie *loader_movie_;

  bool connection_state_;
  QSerialInterface *ser_;

  uint8_t obj_id_;
  int firmware_style_;
  int hardware_type_;
  int electronics_type_;
  uint16_t bootloader_version_;
  uint8_t applications_present_on_motor_;
};

#endif  // CONNECTION_HPP

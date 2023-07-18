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

#include <QStandardPaths>

#include <QMetaEnum>
#include <QMetaObject>

#include <QDateTime>

#define MAJOR_VERSION_MASK 0x000fc000
#define MINOR_VERSION_MASK 0x00003f80
#define PATCH_VERSION_MASK 0x7f

#define MAJOR_VERSION_SHIFT 14
#define MINOR_VERSION_SHIFT 7

#define BOOT_STYLE_MASK 0xfff00000
#define BOOT_MAJOR_MASK 0x000fC000
#define BOOT_MINOR_MASK 0x00003f80
#define BOOT_PATCH_MASK 0x0000007f

#define BOOT_STYLE_SHIFT 20
#define BOOT_MAJOR_SHIFT 14
#define BOOT_MINOR_SHIFT 7

#define UPGRADE_STYLE_MASK 0xfff00000
#define UPGRADE_MAJOR_MASK 0x000fC000
#define UPGRADE_MINOR_MASK 0x00003f80
#define UPGRADE_PATCH_MASK 0x0000007f

#define UPGRADE_STYLE_SHIFT 20
#define UPGRADE_MAJOR_SHIFT 14
#define UPGRADE_MINOR_SHIFT 7

#define HARDWARE_STRING "connected module has hardware type: "
#define ELECTRONICS_STRING "connected module has electronics type: "

class PortConnection : public QObject {
  Q_OBJECT
 public:
  enum ExtraBaudRate{
    Baud921600 = 921600
  };

  bool guessed_module_type_correctly_ = false;

  struct module_connection_values {
      int hardware_value;
      int electronics_value;
  } previous_handled_connection;

  bool logging_active_;

  const QString app_data_folder_ = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  //On windows: C:/Users/jorda/AppData/Local/IQ Control Center
  //On Mac: /Users/iqmotioncontrol/Library/Application Support/IQ Control Center/
  //On linux: /home/iq/.local/share/IQ Control Center/

  const QString path_to_log_file = app_data_folder_ + "/log.txt";
  const QString path_to_user_defaults_repo_ = (app_data_folder_ + "/user_defaults_files");

  static QDateTime time_;

  PortConnection(Ui::MainWindow *user_in);

  ~PortConnection() {}


  QString GetHardwareNameFromResources(int hardware_type);
  void FindHardwareAndElectronicsFromLog(int * hardware_val, int * electronics_val);

  /**
   * @brief ExtractValueFromLog find the most recently added value from the log given the starting character and length of the preamble
   * @return the hardware value
   */
  int ExtractValueFromLog(QString fileLines, int starting_char);

  /**
   * @brief RebootMotor restart the motor
   * @return
   */
  void RebootMotor();

  /**
   * @brief add_to_log write a new line of text to the log
   * @param text_to_log the text to output
   */
  void AddToLog(QString text_to_log);

  /**
   * @brief ShortenLog cut down the number of lines in the file
   */
  void ShortenLog(uint32_t current_num_lines);

  /**
   * @brief getUidValues Get all 3 of the unique ids of the connected motor and return them through pointers
   * @param uid1 pointer to the container for uid1
   * @param uid2 pointer to the container for uid2
   * @param uid3 pointer to the container for uid3
   */
  void GetUidValues(uint32_t * uid1, uint32_t * uid2, uint32_t * uid3);

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
  QSerialInterface *GetQSerialInterface() { return &ser_; }

  /**
   * @brief SetQSerialInterface Sets the serial interface
   * @param setter
   */
  void SetQSerialInterface(QSerialInterface *setter) { ser_ = *setter; }

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
  void SaveNewBootloaderVersion(uint32_t newVersion);

  /**
   * @brief SaveNewUpgraderVersion Sends a command to the motor to save the upgrader version being flashed
   * @param newVersion the raw value of the upgrader version
   */
  void SaveNewUpgraderVersion(uint32_t newVersion);

  /**
   * @brief ResetBootVersionKey Tells the motor whether or not to used the stored boot value
   */
  void ResetBootVersionKey();

  /**
   * @brief ResetUpgradeVersionKey Tells the motor whether or not to use the stored upgrader value
   */
  void ResetUpgradeVersionKey();

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

  /**
   * @brief EnableAllButtons enables all of the pushbuttons on the home screen
   */
  void EnableAllButtons();

  /**
   * @brief DisableAllButtons Disables all of the pushbuttons on the home screen
   */
  void DisableAllButtons();

    void HandleFindingCorrectMotorToRecover(QString detected_module);

 public slots:

  void ConnectMotor();

  void TimerTimeout();

  void PortError(QSerialPort::SerialPortError error);

  void FindPorts();

  void PortComboBoxIndexChanged(int index);

  void BaudrateComboBoxIndexChanged(int index);

  void ClearFirmwareChoices();

 signals:

  void ObjIdFound();

  void FindSavedValues();

  void TypeStyleFound(int, int, int);

  void LostConnection();

 private:
  void DisplayRecoveryMessage();
  void DisplayInvalidFirmwareMessage();
  void GetDeviceInformationResponses();
  int GetFirmwareValid();
  void GetBootAndUpgradeInformation();

  void FillModuleDropdown();

  uint32_t GetLinesInLog();

  Ui::MainWindow *ui_;

  std::string clients_folder_path_ = ":/IQ_api/clients/";
  std::map<std::string, Client *> sys_map_;

  std::vector<QString> ports_names_;
  QString selected_port_name_;
  QString recovery_port_name_;
  qint32 selected_baudrate_;

  QMovie *loader_movie_;

  bool connection_state_;
  QSerialInterface ser_;

  uint8_t obj_id_;
  int firmware_value_;
  int firmware_style_;
  int hardware_type_;
  int electronics_type_;
  uint8_t applications_present_on_motor_;

  QString hardware_str_;
  QString electronics_str_;
};

#endif  // CONNECTION_HPP

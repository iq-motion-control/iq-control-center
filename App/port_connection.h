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
#include "resource_file_handler.h"

#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"

#include "indication_handler.hpp"

#include <QStandardPaths>

#include <QMetaEnum>
#include <QMetaObject>

#include <QDateTime>

#define MAX_MODULE_ID 62

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

#define MODULE_ID_SUB_ID 14
#define MODULE_ID_TYPE_ID 5

#define HARDWARE_STRING "connected module has hardware type: "
#define HARDWARE_MAJOR_STRING "connected module has hardware major version: "
#define ELECTRONICS_STRING "connected module has electronics type: "
#define ELECTRONICS_MAJOR_STRING "connected module has electronics major version: "

class PortConnection : public QObject {
  Q_OBJECT
 public:
  enum ExtraBaudRate{
    Baud921600 = 921600
  };

  bool guessed_module_type_correctly_ = false;

  struct module_connection_values {
      int hardware_type;
      int hardware_major_version;
      int electronics_type;
      int electronics_major_version;
  } previous_handled_connection;

  bool logging_active_;

  const QString app_data_folder_ = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  //On windows: C:/Users/jorda/AppData/Local/IQ Control Center
  //On Mac: /Users/iqmotioncontrol/Library/Application Support/IQ Control Center/
  //On linux: /home/iq/.local/share/IQ Control Center/

  const QString path_to_log_file = app_data_folder_ + "/log.txt";
  const QString path_to_user_defaults_repo_ = (app_data_folder_ + "/user_defaults_files");

  static QDateTime time_;

  PortConnection(Ui::MainWindow *user_in, ResourceFileHandler * resource_file_handler);

  ~PortConnection() {}

  /**
   * @brief Modifies the given type and major version arrays to make sure they are proper matched lists of the same length, to account for backwards compatibility with old metadata files
   * @param hardware_types The hardware types of the modules to get the hardware names for.
   * @param hardware_major_versions The hardware major versions of the modules to get the hardware names for.
   * @param electronics_types The electronics types of the modules to get the hardware names for.
   * @param electronics_major_versions The electronics major versions of the modules to get the hardware name for.
   * @return None
   */
  void SanitizeHardwareAndElectronicsLists(QJsonArray &hardware_types, QJsonArray &hardware_major_versions, QJsonArray &electronics_types, QJsonArray &electronics_major_versions);

  /**
   * @brief GetHardwareNameFromResources given the hardware type and version and electronics type and version, go into our resource files and grab out the module name
   * @param hardware_type The hardware type of the module to get the hardware name for
   * @param hardware_major_version The hardware major version of the module to get the hardware name for
   * @param electronics_type The electronics type of the module to get the hardware name for
   * @param electronics_major_version The electronics major version of the module to get the hardware name for
   * @return The name of the module with hardware_type value (Ex. 30 would return Vertiq 4006 370kv)
   */
  QString GetHardwareNameFromResources(int hardware_type, int hardware_major_version, int electronics_type, int electronics_major_version);

  /**
   * @brief GetHardwareNameFromResources given the hardware types and versions and electronics types and versions, go into our resource files and grab out all the module names specified by those paired lists.
   * @param hardware_types The hardware types of the modules to get the hardware names for. Paired with hardware_major_versions
   * @param hardware_major_versions The hardware major versions of the modules to get the hardware names for. Paired with hardware_types
   * @param electronics_types The electronics types of the modules to get the hardware names for. Paired with electronics_major_versions
   * @param electronics_major_versions The electronics major versions of the modules to get the hardware name for. Paired with electronics_types
   * @return The names of all possible modules with the combinations of types and major versions specified by these arrays.
   */
  QString GetHardwareNameFromResources(QJsonArray hardware_types, QJsonArray hardware_major_versions, QJsonArray electronics_types, QJsonArray electronics_major_versions);

  /**
   * @brief FindHardwareAndElectronicsFromLog Go into the persistent log, and find the electronics and hardware type and major version of the most recent connection
   * @param hardware_type A pointer to hold the hardware type
   * @param hardware_major_version A pointer to hold the hardware major version
   * @param electronics_type A pointer to hold the electronics type
   * @param electronics_major_version A pointer to hold the electronics major version
   */
  void FindHardwareAndElectronicsFromLog(int * hardware_type, int* hardware_major_version, int * electronics_type, int * electronics_major_version);

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
   * @brief Clear firmware, hardware, bootloader, and upgrader values from the Information Panel
   */
  void EraseInformationPanel();

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
   * @return int hardware type
   */
  int GetHardwareType() { return hardware_type_; }
  /**
   * @brief SetHardwareType
   * @param setter
   */
  void SetHardwareType(const int &setter) { hardware_type_ = setter; }

  /**
   * @brief GetHardwareMajorVersion
   * @return int hardware major version
   */
  int GetHardwareMajorVersion() { return hardware_major_version_; }

  /**
   * @brief SetHardwareMajorVersion
   * @param setter
   */
  void SetHardwareMajorVersion(const int &setter) { hardware_major_version_ = setter; }

  /**
   * @brief GetElectronicsType
   * @return int electronics type
   */
  int GetElectronicsType() { return electronics_type_; }

  /**
   * @brief GetElectronicsMajorVersion
   * @return int electronics major version
   */
  int GetElectronicsMajorVersion() { return electronics_major_version_; }

  /**
   * @brief SetElectronicsMajorVersion
   * @param setter
   */
  void SetElectronicsMajorVersion(const int &setter) { electronics_major_version_ = setter; }

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

  /**
   * @brief HandleFindingCorrectMotorToRecover given our detected module input, give the user the chance to tell us that we assumed correctly or not
   * @param detected_module the string describing the module we're assuming the user is recovering
   */
  void HandleFindingCorrectMotorToRecover(QString detected_module);

  std::string GetClentsFolderPath();

  /**
   * @brief ConnectMotor populates all tabs based on the information gotten from the motor
   */
  void ConnectMotor();

  uint8_t GetSysMapObjId();

  std::map<std::string, Client *> GetSystemControlMap();

  bool ModuleIdAlreadyExists(uint8_t module_id);

  void ClearDetections();

  void HandleRestartNeeded();

 public slots:
  void DetectModulesClickedCallback();

  void ConnectToSerialPort();

  void TimerTimeout();

  void PortError(QSerialPort::SerialPortError error);

  void FindPorts();

  void PortComboBoxIndexChanged(int index);

  void BaudrateComboBoxIndexChanged(int index);

  void ModuleIdComboBoxIndexChanged(int index);

  void ClearFirmwareChoices();

  void PlayIndication();

 signals:

  void ObjIdFound();

  void FindSavedValues();

  void TypeStyleFound(int hardware_type, int hardware_major_version, int electronics_type, int electronics_major_version, int firmware_style, int firmware_value);

  void LostConnection();

 private:
  void DetectNumberOfModulesOnBus();
  bool DisplayRecoveryMessage();
  void DisplayInvalidFirmwareMessage();
  void GetDeviceInformationResponses();
  int GetFirmwareValid();
  void GetBootAndUpgradeInformation();

  void UpdateGuiWithModuleIds(uint8_t module_id_with_different_sys_control);

  uint32_t GetLinesInLog();

  Ui::MainWindow *ui_;

  //Object that handles finding and extracting information from resource files. Useful for extracting names during recovery and flash checking.
  ResourceFileHandler * resource_file_handler_;

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
  uint8_t system_control_id_;

  int firmware_value_;
  int firmware_style_;
  int hardware_type_;
  int hardware_major_version_;
  int electronics_type_;
  int electronics_major_version_;
  uint8_t applications_present_on_motor_;

  QString hardware_str_;
  QString hardware_major_str_;
  QString electronics_str_;
  QString electronics_major_str_;

  uint8_t detected_module_ids_[MAX_MODULE_ID + 1]; //We can have a maximum of 63 modules before we run out of possible module IDs [0, 62]
  uint8_t num_modules_discovered_; //keep track of the number we've actually found

  IndicationHandler indication_handle_;

  bool perform_timer_callback_;
};

#endif  // CONNECTION_HPP

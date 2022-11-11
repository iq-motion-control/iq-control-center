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

#include "port_connection.h"

PortConnection::PortConnection(Ui::MainWindow *user_int) : ui_(user_int), ser_(nullptr) {
  SetPortConnection(0);
  sys_map_ = ClientsFromJson(0, "system_control_client.json", clients_folder_path_);
}

void PortConnection::SetPortConnection(bool state) {
  if (state) {
    connection_state_ = 1;
    ui_->header_connect_button->setText("DISCONNECT");
  } else {
    connection_state_ = 0;
    ports_names_.clear();
    selected_port_name_.clear();

    ui_->header_combo_box->clear();

    ui_->label_firmware_build_value->setText(QString(""));
    ui_->label_firmware_name->setText(QString(""));
    ui_->label_hardware_name->setText(QString(""));
    ui_->label_bootloader_value->setText(QString(""));
    ui_->label_upgrader_value->setText(QString(""));

    ui_->header_connect_button->setText("CONNECT");
  }
}

void PortConnection::ConnectMotor() {
  if (connection_state_ == 0) {
    if (!selected_port_name_.isEmpty()) {
      QString message = "Connecting . . . ";
      ui_->header_error_label->setText(message);
      ui_->header_error_label->repaint();

      connection_state_ = 0;

      uint32_t firmware_value = 0;
      uint32_t hardware_value = 0;
      uint8_t applications_on_motor = 0;
      int firmware_valid = 0;
      int hardware_type = 1;
      int firmware_style = 1;
      int electronics_value = 1;
      int electronics_type = 1;
      int bootloader_value = 0;
      int upgrade_value = 0;

      //Update the firmware build number to work with Major, Minor, Patch
      int firmware_build_major = 0;
      int firmware_build_minor = 0;
      int firmware_build_patch = 0;

      //Upgrade version has style, major, minor, patch (style is what kind of upgrade)
      int upgrade_major = 0;
      int upgrade_minor = 0;
      int upgrade_patch = 0;

      //Bootloader version
      int boot_major = 0;
      int boot_minor = 0;
      int boot_patch = 0;

      ser_ = new QSerialInterface(selected_port_name_, selected_baudrate_);
      try {
        if (!ser_->ser_port_->open(QIODevice::ReadWrite))
          throw QString("CONNECTION ERROR: could not open serial port");

        if (!GetEntryReply(*ser_, sys_map_["system_control_client"], "module_id", 5, 0.05f,
                           obj_id_))
          throw QString(
              "CONNECTION ERROR: please check selected port and baudrate or reconnect IQ module");

        emit ObjIdFound();

        // checks if new firmware is avaible, otherwise defaults to speed and hardware type 1;
        if (GetEntryReply(*ser_, sys_map_["system_control_client"], "hardware", 5, 0.05f,
                          hardware_value)) {
          if (GetEntryReply(*ser_, sys_map_["system_control_client"], "firmware", 5, 0.05f,
                             firmware_value))
              if (GetEntryReply(*ser_, sys_map_["system_control_client"], "bootloader_version", 5, 0.05f,
                                 bootloader_value))
                  if (GetEntryReply(*ser_, sys_map_["system_control_client"], "upgrade_version", 5, 0.05f,
                                     upgrade_value))
                      if(!GetEntryReply(*ser_, sys_map_["system_control_client"], "electronics", 5, 0.05f,
                                        electronics_value))
                     throw QString("CONNECTION ERROR: please check selected port or reconnect IQ Module");

          //Firmware value holds the raw 32 bits of firmware information
          //This information comes from the Product ID Convention sheet
          hardware_type = (hardware_value >> 16);
          firmware_style = (firmware_value >> 20);
          electronics_type = electronics_value >> 16;

          firmware_build_major = (firmware_value & MAJOR_VERSION_MASK) >> MAJOR_VERSION_SHIFT;
          firmware_build_minor = (firmware_value & MINOR_VERSION_MASK) >> MINOR_VERSION_SHIFT;
          firmware_build_patch = firmware_value & PATCH_VERSION_MASK;

          boot_major = (bootloader_value & BOOT_MAJOR_MASK) >> BOOT_MAJOR_SHIFT;
          boot_minor = (bootloader_value & BOOT_MINOR_MASK) >> BOOT_MINOR_SHIFT;
          boot_patch = bootloader_value & BOOT_PATCH_MASK;

          upgrade_major = (upgrade_value & BOOT_MAJOR_MASK) >> BOOT_MAJOR_SHIFT;
          upgrade_minor = (upgrade_value & BOOT_MINOR_MASK) >> BOOT_MINOR_SHIFT;
          upgrade_patch = upgrade_value & BOOT_PATCH_MASK;

          if(!GetEntryReply(*ser_, sys_map_["system_control_client"], "firmware_valid", 5, 0.05f,
                            firmware_valid))
            throw(QString("FIRMWARE ERROR: unable to determine firmware validity"));
        }

        firmware_style_ = firmware_style;
        hardware_type_ = hardware_type;
        electronics_type_ = electronics_type;

        QString firmware_build_number_string = QString::number(firmware_build_major) + "." + QString::number(firmware_build_minor) + "." + QString::number(firmware_build_patch);

        ui_->label_firmware_build_value->setText(firmware_build_number_string);

        //If we have a bootloader label its version, otherwise put N/A
        QString bootloader_version_string = "";
        if(bootloader_value != 0){
            bootloader_version_string = QString::number(boot_major) + "." + QString::number(boot_minor) + "." + QString::number(boot_patch);
        }else{
            bootloader_version_string = "N/A";
        }
        ui_->label_bootloader_value->setText(bootloader_version_string);

        //If we have an upgrader label its version, otherwise put N/A
        QString upgrade_version_string = "";
        if(bootloader_value != 0){
            upgrade_version_string = QString::number(upgrade_major) + "." +QString::number(upgrade_minor) + "." +QString::number(upgrade_patch);
        }else{
            upgrade_version_string = "N/A";
        }
        ui_->label_upgrader_value->setText(upgrade_version_string);


        SetPortConnection(1);
        QString message = "Motor connected Successfully";
        ui_->header_error_label->setText(message);

        if(!firmware_valid){
          QMessageBox msgBox;
          msgBox.setWindowTitle("WARNING!");
          msgBox.setText(
              "Invalid Firmware has been loaded onto the Connected IQ Module\n\n"
              "Please Flash valid Firmware to avoid damage such as fires or explosions"
              "to the IQ Module. \n\nValid Firmware can be found at www.iq-control.com/support");
          msgBox.setStandardButtons(QMessageBox::Ok);
          msgBox.setDefaultButton(QMessageBox::Ok);
          msgBox.exec();
        }

        //If we have valid firmware, check what applications are on the motor to use later with populating flash buttons
        //If you don't get a response, this motor doesn't know how to deal with this entry. Give back 0
        if(!GetEntryReply(*ser_, sys_map_["system_control_client"], "apps_present", 5, 0.05f,
                          applications_on_motor)){
            applications_present_on_motor_ = 0;
        }else{
            applications_present_on_motor_ = applications_on_motor;
        }

        emit TypeStyleFound(hardware_type_, firmware_style_, firmware_value);
        emit FindSavedValues();
      } catch (const QString &e) {
        ui_->header_error_label->setText(e);
        delete ser_->ser_port_;
        SetPortConnection(0);
      }
    } else {
      QString error_message = "NO PORT SELECTED: please select a port from the drop down menu";
      ui_->header_error_label->setText(error_message);
      SetPortConnection(0);
    }
  } else if (connection_state_ == 1) {
    delete ser_->ser_port_;
    SetPortConnection(0);
    emit LostConnection();
  }
}

void PortConnection::TimerTimeout() {
  if (connection_state_ == 1) {
    uint8_t obj_id;
    if (!GetEntryReply(*ser_, sys_map_["system_control_client"], "module_id", 5, 0.05f, obj_id)) {
      delete ser_->ser_port_;
      SetPortConnection(0);
      QString error_message = "Serial Port Disconnected";
      ui_->header_error_label->setText(error_message);
      emit LostConnection();
    }
  }
}

void PortConnection::PortError(QSerialPort::SerialPortError error) {
  if (error == 9) {
    SetPortConnection(0);
    delete ser_->ser_port_;
    QString error_message = "Serial Port Disconnected";
    ui_->header_error_label->setText(error_message);
    emit LostConnection();
  }
}

void PortConnection::PortComboBoxIndexChanged(int index) {
  selected_port_name_ = ports_names_[index];
}

void PortConnection::FindPorts() {
  ports_names_.clear();
  selected_port_name_.clear();
  ui_->header_combo_box->clear();
  Q_FOREACH (QSerialPortInfo port_info, QSerialPortInfo::availablePorts()) {
    ports_names_.push_back(port_info.portName());
    ui_->header_combo_box->addItem(port_info.portName());
  }
  if (ports_names_.size() != 0) {
    PortComboBoxIndexChanged(0);
  }
}

void PortConnection::BaudrateComboBoxIndexChanged(int index) {
  selected_baudrate_ = ui_->header_baudrate_combo_box->currentData().value<int>();
}

void PortConnection::FindBaudrates() {
  ui_->header_baudrate_combo_box->clear();
  //QSerialPort only goes up to 115200, and its a back-end library, not part of our project, so best not to modify it. Add our own extra baud rate enum to port_connection to cover this and future cases
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("921600"), Baud921600);
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
  selected_baudrate_ = ui_->header_baudrate_combo_box->currentData().value<int>();
}

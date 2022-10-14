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
      int firmware_valid = 0;
      int hardware_type = 1;
      int firmware_style = 1;

      //Update the firmware build number to work with Major, Minor, Patch
      int firmware_build_major = 0;
      int firmware_build_minor = 0;
      int firmware_build_patch = 0;

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
          if (!GetEntryReply(*ser_, sys_map_["system_control_client"], "firmware", 5, 0.05f,
                             firmware_value))
            throw QString("CONNECTION ERROR: please check selected port or reconnect IQ Module");

          //Firmware value holds the raw 32 bits of firmware information
          hardware_type = (hardware_value >> 16);
          firmware_style = (firmware_value >> 20);

          firmware_build_major = (firmware_value & MAJOR_VERSION_MASK) >> 14;
          firmware_build_minor = (firmware_value & MINOR_VERSION_MASK) >> 7;
          firmware_build_patch = firmware_value & PATCH_VERSION_MASK;

          if(!GetEntryReply(*ser_, sys_map_["system_control_client"], "firmware_valid", 5, 0.05f,
                            firmware_valid))
            throw(QString("FIRMWARE ERROR: unable to determine firmware validity"));
        }

        firmware_style_ = firmware_style;
        hardware_type_ = hardware_type;

        QString firmware_build_number_string = QString::number(firmware_build_major) + "." + QString::number(firmware_build_minor) + "." + QString::number(firmware_build_patch);

        ui_->label_firmware_build_value->setText(firmware_build_number_string);

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
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
  ui_->header_baudrate_combo_box->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
  selected_baudrate_ = ui_->header_baudrate_combo_box->currentData().value<int>();
}

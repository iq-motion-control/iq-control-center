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
#include "schmi/include/Schmi/qserial.h"

#include <QStandardPaths>

#define MAXIMUM_LINES_IN_LOG_FILE 50000 //Using the 4006 as the example, we have ~85 lines/connection -> ~580 connections before delete. Don't want to go
                                        //too much bigger because it slows the program down when we read the size
#define LINES_TO_REMOVE_FROM_LOG_ON_OVERFLOW 250

PortConnection::PortConnection(Ui::MainWindow *user_int) :  logging_active_(false), ui_(user_int), ser_(nullptr) {
  SetPortConnection(0);
  sys_map_ = ClientsFromJson(0, "system_control_client.json", clients_folder_path_, nullptr, nullptr);
}

void PortConnection::AddToLog(QString text_to_log){
    if(logging_active_){
        //Get a string of the line we want to write to the log
        QString logMessage(time_.currentDateTime().toString(Qt::TextDate) + ": " + text_to_log + "\n");

        //Add that line to the GUI
        //make sure the text cursor is at the end of the document before writing the new line
        ui_->log_text_browser->moveCursor(QTextCursor::End);
        ui_->log_text_browser->insertPlainText(logMessage);

        //Write the same line to the persistent log file that exists in the background
        QFile log_file(path_to_log_file);
        log_file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream iStream( &log_file );
        iStream.setCodec( "utf-8" );
        iStream << logMessage;

        uint32_t lines_in_log = GetLinesInLog();

        if(lines_in_log > MAXIMUM_LINES_IN_LOG_FILE){
            log_file.close();
            ShortenLog();

            return;
        }

        log_file.close();
    }
}

uint32_t PortConnection::GetLinesInLog(){
    uint32_t lines = 0;

    QFile log_file(path_to_log_file);

    if(log_file.open(QIODevice::ReadOnly)){
        while(!log_file.atEnd()){
            log_file.readLine();
            lines++;
        }
    }

    //Reset the file
    log_file.seek(0);
    log_file.close();

    return lines;
}

void PortConnection::ShortenLog(){

    //Open up the log file with all write permissions available so we can delete it
    QFile log_file(path_to_log_file);
    if (!log_file.open(QIODevice::ReadWrite | QIODevice::Text)){
        return;
    }

    log_file.setPermissions(QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther);

    //Create a string that will hold the data we'll write to the new log
    QString newLog;

    uint32_t curLine = 0;
    //Go through the current log, but only grab the last LINES_TO_REMOVE_FROM_LOG lines
    while(!log_file.atEnd()){
        //once we get to past the first lines we don't want to include anymore
        if(curLine >= LINES_TO_REMOVE_FROM_LOG_ON_OVERFLOW){
            QByteArray newLine = log_file.readLine();
            newLog.append(QString(newLine));
        }else{
            //Just move along in the file
            log_file.readLine();
        }

        curLine++;
    }

    //Delete the current log
    log_file.remove();

    //Make a new log with only the lines we want
    QFile newLogFile(path_to_log_file);
    newLogFile.open(QIODevice::WriteOnly);
    newLogFile.write(newLog.toUtf8());
    newLogFile.close(); //don't forget to close your files!
}

void PortConnection::ResetToTopPage(){
    ui_->stackedWidget->setCurrentIndex(0);
}

int PortConnection::GetCurrentTab(){
    return ui_->stackedWidget->currentIndex();
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

    AddToLog("module disconnected");
  }
}

void PortConnection::ConnectMotor() {
  if (connection_state_ == 0) {
    if (!selected_port_name_.isEmpty()) {
      QString message = "Connecting . . . ";
      ui_->header_error_label->setText(message);
      ui_->header_error_label->repaint();

      connection_state_ = 0;

      int firmware_valid = 0;

      ser_.InitSerial(selected_port_name_, selected_baudrate_);
      try {

        //Try to open the serial port in general
        if (!ser_.ser_port_->open(QIODevice::ReadWrite)){
          throw QString("CONNECTION ERROR: could not open serial port");
        }

        //Before we try to connect with iquart, let's check if we are in the ST bootloader (recovery mode)
        if(CheckIfInBootLoader()){
            DisplayRecoveryMessage();
        }

        //Check if the firmware is valid
        firmware_valid = GetFirmwareValid();

        SetPortConnection(1);
        QString message = "Module Connected Successfully";

        //Write the fact that we connected with the motor to the output log
        AddToLog("New Module Connected");
        AddToLog(message.toLower() + " on " + selected_port_name_ + " at " + QString::number(selected_baudrate_) + " baud\n");
        AddToLog("Module variables follow:");

        logging_active_ = false;

        ui_->header_error_label->setText(message);

        if(!firmware_valid){
            DisplayInvalidFirmwareMessage();
            message = "Motor Connected Successfully. Please Flash Valid Firmware";
            ui_->header_error_label->setText(message);
            ui_->label_firmware_build_value->setText("");
        //If we have valid firmware, we can go ahead and grab all of the data, if not, don't try
        }else{
            //Get information about what firmware is on the motor
            logging_active_ = true;
            GetDeviceInformationResponses();
            GetBootAndUpgradeInformation();
            logging_active_ = false;

            //Send out the hardware and firmware values to other modules of Control Center
            emit TypeStyleFound(hardware_type_, firmware_style_, firmware_value_);

            /**
             * So...both emitting TypeStyleFound and FindSavedValues end up asking the motor for saved values (though the former
             * never gets at the advanced values. I haven't figured that out yet, but it's another one of thos legacy things that
             * I don't want to touch too much yet. Anyway, I only want to log when we get all of the values from the module, so we
             * turn off logging after we say "we connected" and turn it back on when we go through and get all of the saved values"
            */
            logging_active_ = true;
            emit FindSavedValues();
        }

      } catch (const QString &e) {
        ui_->header_error_label->setText(e);
        delete ser_.ser_port_;
        SetPortConnection(0);
      }
    } else {
      QString error_message = "NO PORT SELECTED: please select a port from the drop down menu";
      ui_->header_error_label->setText(error_message);
      SetPortConnection(0);
    }
  } else if (connection_state_ == 1) {
    ClearFirmwareChoices();
    delete ser_.ser_port_;
    SetPortConnection(0);
    emit LostConnection();
  }
}

void PortConnection::ClearFirmwareChoices(){
    ui_->flash_boot_button->setVisible(false);
    ui_->flash_app_button->setVisible(false);
    ui_->flash_button->setVisible(false);
    ui_->flash_upgrade_button->setVisible(false);
    ui_->select_firmware_binary_button->setText("Select Firmware (\".bin\") or (\".zip\")");
    ui_->select_recovery_bin_button->setText("Select Firmware (\".bin\") or (\".zip\")");
}

void PortConnection::DisableAllButtons(){
    ui_->pushButton_home->setEnabled(false);
    ui_->pushButton_advanced->setEnabled(false);
    ui_->pushButton_firmware->setEnabled(false);
    ui_->pushButton_testing->setEnabled(false);
    ui_->pushButton_tuning->setEnabled(false);
    ui_->pushButton_general->setEnabled(false);
}

void PortConnection::EnableAllButtons(){
    ui_->pushButton_home->setEnabled(true);
    ui_->pushButton_advanced->setEnabled(true);
    ui_->pushButton_firmware->setEnabled(true);
    ui_->pushButton_testing->setEnabled(true);
    ui_->pushButton_tuning->setEnabled(true);
    ui_->pushButton_general->setEnabled(true);
}

void PortConnection::DisplayRecoveryMessage(){
    recovery_port_name_ = selected_port_name_;
    ser_.ser_port_->close();
    QMessageBox msgBox;
    msgBox.setWindowTitle("Recovery Mode Recognized");
    msgBox.setText(
        "It appears that your motor is currently in recovery mode.\n"
        "Would you like to recover your motor now?");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    if (msgBox.exec() == QMessageBox::Yes) {
      DisableAllButtons();
      ui_->stackedWidget->setCurrentIndex(6);

      AddToLog("motor connected in recovery mode\n");

    }else{
      ui_->pushButton_home->setChecked(true);
      ui_->stackedWidget->setCurrentIndex(0);
    }

    throw QString("Recovery Detected");
}

void PortConnection::GetDeviceInformationResponses(){
    uint32_t hardware_value = 0;
    uint32_t firmware_value = 0;
    int electronics_value = 1;

    int hardware_type = 1;
    int firmware_style = 1;
    int electronics_type = 1;

    //Update the firmware build number to work with Major, Minor, Patch
    int firmware_build_major = 0;
    int firmware_build_minor = 0;
    int firmware_build_patch = 0;

    if (!GetEntryReply(ser_, sys_map_["system_control_client"], "module_id", 5, 0.05f, obj_id_)){
        throw QString(
        "CONNECTION ERROR: please check selected port and baudrate or reconnect IQ module");
    }

    emit ObjIdFound();

    // checks if new firmware is avaible, otherwise defaults to speed and hardware type 1;
    if (GetEntryReply(ser_, sys_map_["system_control_client"], "hardware", 5, 0.05f,
                      hardware_value)){
      if (GetEntryReply(ser_, sys_map_["system_control_client"], "firmware", 5, 0.05f,
                         firmware_value)){
          if(!GetEntryReply(ser_, sys_map_["system_control_client"], "electronics", 5, 0.05f,
                            electronics_value)){

                    throw QString("CONNECTION ERROR: please check selected port or reconnect IQ Module");
            }
        }
     }

    //Firmware value holds the raw 32 bits of firmware information
    //This information comes from the Product ID Convention sheet
    hardware_type = (hardware_value >> 16);
    firmware_style = (firmware_value >> 20);
    electronics_type = electronics_value >> 16;

    firmware_build_major = (firmware_value & MAJOR_VERSION_MASK) >> MAJOR_VERSION_SHIFT;
    firmware_build_minor = (firmware_value & MINOR_VERSION_MASK) >> MINOR_VERSION_SHIFT;
    firmware_build_patch = firmware_value & PATCH_VERSION_MASK;

    //Create a string in the 'x.x.x' format
    QString firmware_build_number_string = QString::number(firmware_build_major) + "." + QString::number(firmware_build_minor) + "." + QString::number(firmware_build_patch);
    ui_->label_firmware_build_value->setText(firmware_build_number_string);

    uint32_t uid1, uid2, uid3;
    GetUidValues(&uid1, &uid2, &uid3);
    AddToLog("UID1: " + QString::number(uid1) + " UID2: " + QString::number(uid2) + " UID3: " + QString::number(uid3));
    AddToLog("module connected has build version: " + firmware_build_number_string);

    firmware_value_ = firmware_value;
    firmware_style_ = firmware_style;
    hardware_type_ = hardware_type;
    electronics_type_ = electronics_type;
}

int PortConnection::GetFirmwareValid(){
    int firmware_valid;
    //Check to see if the firmware is valid
    if(!GetEntryReply(ser_, sys_map_["system_control_client"], "firmware_valid", 5, 0.05f,
                      firmware_valid)){
      AddToLog("FIRMWARE ERROR: unable to determine firmware validity. Is your module powered on?");
      throw(QString("FIRMWARE ERROR: unable to determine firmware validity. Is your module powered on?"));
    }

    return firmware_valid;
}

void PortConnection::GetBootAndUpgradeInformation(){

    uint8_t applications_on_motor = 0;

    //Upgrade version has style, major, minor, patch (style is what kind of upgrade)
    int upgrade_value = -1;
    int upgrade_style = 0;
    int upgrade_major = 0;
    int upgrade_minor = 0;
    int upgrade_patch = 0;

    //Bootloader version
    int bootloader_value = -1;
    int boot_style = 0;
    int boot_major = 0;
    int boot_minor = 0;
    int boot_patch = 0;

    //If we have valid firmware, check what applications are on the motor to use later with populating flash buttons
    //If you don't get a response, this motor doesn't know how to deal with this entry. Give back 0
    if(!GetEntryReply(ser_, sys_map_["system_control_client"], "apps_present", 5, 0.05f,
                      applications_on_motor)){
        applications_present_on_motor_ = 0;
    }else{
        applications_present_on_motor_ = applications_on_motor;
    }

    //Grab the bootloader and upgrade versions if they exist
    GetEntryReply(ser_, sys_map_["system_control_client"], "bootloader_version", 5, 0.05f, bootloader_value);
    GetEntryReply(ser_, sys_map_["system_control_client"], "upgrade_version", 5, 0.05f, upgrade_value);

    //Parse out the important data
    boot_style = (bootloader_value & BOOT_STYLE_MASK) >> BOOT_STYLE_SHIFT;
    boot_major = (bootloader_value & BOOT_MAJOR_MASK) >> BOOT_MAJOR_SHIFT;
    boot_minor = (bootloader_value & BOOT_MINOR_MASK) >> BOOT_MINOR_SHIFT;
    boot_patch = bootloader_value & BOOT_PATCH_MASK;

    upgrade_style = (upgrade_value & UPGRADE_STYLE_MASK) >> UPGRADE_STYLE_SHIFT;
    upgrade_major = (upgrade_value & UPGRADE_MAJOR_MASK) >> UPGRADE_MAJOR_SHIFT;
    upgrade_minor = (upgrade_value & UPGRADE_MINOR_MASK) >> UPGRADE_MINOR_SHIFT;
    upgrade_patch = upgrade_value & UPGRADE_PATCH_MASK;

    //If the motor does not have a way to report back the boot or upgrade version, it will send back -1
    //If the motor has a way to report back the boot or upgrade, but is not using it, the value will be 0. We should not report 0
    bool bootloader_in_use = ((bootloader_value != -1) && (bootloader_value != 0));
    bool upgrader_in_use = ((upgrade_value != -1) && (upgrade_value != 0));

    //If we have a bootloader label its version, otherwise put N/A
    QString bootloader_version_string = "";
    if(bootloader_in_use){
        bootloader_version_string = QString::number(boot_style) + "." + QString::number(boot_major) + "." + QString::number(boot_minor) + "." + QString::number(boot_patch);
    }else{
        bootloader_version_string = "N/A";
    }
    ui_->label_bootloader_value->setText(bootloader_version_string);
    AddToLog("module connected has bootloader version: " + bootloader_version_string);


    //If we have an upgrader label its version, otherwise put N/A
    QString upgrade_version_string = "";
    if(upgrader_in_use){
        upgrade_version_string = QString::number(upgrade_style) + "." + QString::number(upgrade_major) + "." +QString::number(upgrade_minor) + "." +QString::number(upgrade_patch);
    }else{
        upgrade_version_string = "N/A";
    }
    ui_->label_upgrader_value->setText(upgrade_version_string);
    AddToLog("module connected has upgrade version: " + upgrade_version_string);
}

void PortConnection::DisplayInvalidFirmwareMessage(){
    QMessageBox msgBox;
    msgBox.setWindowTitle("WARNING!");
    msgBox.setText(
        "Invalid firmware has been loaded onto the connected IQ Module.\n\n"
        "Please flash valid firmware to avoid damage (such as fires or explosions)"
        " to the IQ Module. \n\nValid Firmware can be found at www.vertiq.co/support");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void PortConnection::TimerTimeout() {
  if (connection_state_ == 1) {
    uint8_t obj_id;
    if (!GetEntryReply(ser_, sys_map_["system_control_client"], "module_id", 5, 0.05f, obj_id)) {
      delete ser_.ser_port_;
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
    delete ser_.ser_port_;
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

bool PortConnection::CheckIfInBootLoader(){
    //use https://www.st.com/resource/en/application_note/an3155-usart-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf
    //As our guide to know if we are in the bootloader

    //If we have already sent the initial command, we can send any of the commands available in the bootloader
    //If we get the expected response, we still know we're in the bootloader
    #define INIT_CMD 0x7f
    #define ACK 0x79
    #define NACK 0x1f

    uint8_t sendBuffer[] = {INIT_CMD};
    uint8_t readBuf[1];

    //We want to flash at 115200 Baud always. So let's ping the bootloader at 115200
    //so that we init to the right value when we go to the flash loader
    ser_.ser_port_->setBaudRate(Schmi::SerialConst::BAUD_RATE);

    //Send the first byte the STM32 bootloader expects (0x7f). If we get a response and it's
    //0x79 then we know we're in the st bootloader
    //The response can also be a NACK (0x1F) if we send this once the bootloader is already intialized
    //Try multiple times to account for any instability (in testing we successfully get a NACK every other try)
    const int RETRIES = 5;
    for(int i = 0; i < RETRIES; i++){
        ser_.Write(&sendBuffer[0], 1);
        ser_.Read(readBuf, 1, 100);
        if(readBuf[0] == ACK || readBuf[0] == NACK){
            return true;
        }
    }
    //If we didn't get a response, reset to the baud rate we had when we called this
    ser_.ser_port_->setBaudRate(selected_baudrate_);
    return false;
}

void PortConnection::GetUidValues(uint32_t * uid1, uint32_t * uid2, uint32_t * uid3){
    GetEntryReply(ser_, sys_map_["system_control_client"], "uid1", 5, 0.05f, *uid1);
    GetEntryReply(ser_, sys_map_["system_control_client"], "uid2", 5, 0.05f, *uid2);
    GetEntryReply(ser_, sys_map_["system_control_client"], "uid3", 5, 0.05f, *uid3);
}

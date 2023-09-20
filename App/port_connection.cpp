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
#include "iq_flasher/include/Schmi/qserial.h"
#include "metadata_handler.hpp"
#include <QStandardPaths>

#define MAXIMUM_LINES_IN_LOG_FILE 50000 //Using the 4006 as the example, we have ~85 lines/connection -> ~580 connections before delete. Don't want to go
                                        //too much bigger because it slows the program down when we read the size

#define DEFAULT_OBJECT_ID 0
#define NO_OLD_MODULES_FOUND 99

PortConnection::PortConnection(Ui::MainWindow *user_int) :
  logging_active_(false),
  ui_(user_int),
  ser_(nullptr),
  hardware_str_(HARDWARE_STRING),
  electronics_str_(ELECTRONICS_STRING),
  num_modules_discovered_(0),
  indication_handle_(&ser_, clients_folder_path_),
  perform_timer_callback_(true)
{

      SetPortConnection(0);

      sys_map_ = ClientsFromJson(DEFAULT_OBJECT_ID, "system_control_client.json", clients_folder_path_, nullptr, nullptr);

      //init these to a known value so we know what to do on an attempt to connect to a recovery mode module
      hardware_type_ = -1;
      electronics_type_ = -1;
}

void PortConnection::AddToLog(QString text_to_log){
    if(logging_active_){
        QString logMessage;

        if(text_to_log == "\n"){
            logMessage = "\n";
        }else{
            //Get a string of the line we want to write to the log
            logMessage = time_.currentDateTime().toString(Qt::TextDate) + ": " + text_to_log + "\n";
        }

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

void PortConnection::ShortenLog(uint32_t current_num_lines){

    //Only delete the number of lines needed to get back to the max allowed
    uint32_t lines_to_delete = current_num_lines - MAXIMUM_LINES_IN_LOG_FILE;

    //Open up the log file with all write permissions available so we can delete it
    QFile log_file(path_to_log_file);
    if (!log_file.open(QIODevice::ReadWrite | QIODevice::Text)){
        return;
    }

    log_file.setPermissions(QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteGroup | QFileDevice::WriteOther);

    //Create a string that will hold the data we'll write to the new log
    QString newLog;

    uint32_t curLine = 0;
    //Go through the current log and kill the lines causing the overfill
    while(!log_file.atEnd()){
        //once we get to past the first lines we don't want to include anymore
        if(curLine > lines_to_delete){
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

void PortConnection::FindHardwareAndElectronicsFromLog(int * hardware_val, int * electronics_val){
    QString fileLines;

    QFile file(path_to_log_file);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        while (!file.atEnd()) {
            fileLines.append(file.readLine());
        }
    }

    //lastIndexOf returns -1 if it can't find the string
    int last_hardware_instance_index = fileLines.lastIndexOf(hardware_str_);
    int last_electronics_instance_index = fileLines.lastIndexOf(electronics_str_);

    //so long as we can find the strings in the log we can extract the values. otherwise they need to be -1
    //to indicate an issue
    if(last_hardware_instance_index != -1 && last_electronics_instance_index != -1){
        int hardware_starting_char =  last_hardware_instance_index + hardware_str_.size();
        int electronics_starting_char = last_electronics_instance_index + electronics_str_.size();

        *hardware_val =  ExtractValueFromLog(fileLines, hardware_starting_char);
        *electronics_val =  ExtractValueFromLog(fileLines, electronics_starting_char);

        AddToLog("Found last connection from log. Hardware: " + QString::number(*hardware_val) + " Electronics: " + QString::number(*electronics_val));

        return;
    }

    //If we couldn't find any connection instances, spit back invalid numbers
    *hardware_val =  -1;
    *electronics_val = -1;
    AddToLog("Could not find any previous connections in the log");
}

int PortConnection::ExtractValueFromLog(QString fileLines, int starting_char){
    //unicode defines the values '0'-'9' from 0x0030 - 0x0039 inclusive (same as ascii)
    QString value;

    int current_char = starting_char;

    //Tested that this will work with any length of integer (tested from 1 - 1234)
    //Just keep going until the character you're on isn't an integer anymore
    while(fileLines.at(current_char) >= int('0') && fileLines.at(current_char) <= int('9')){
        value.append(fileLines.at(current_char));
        current_char++;
    }

    return value.toInt();
}

void PortConnection::ResetToTopPage(){
    //Make sure that the home button is the only one that's checked
    ui_->stackedWidget->setCurrentIndex(0);
    ui_->pushButton_advanced->setChecked(false);
    ui_->pushButton_general->setChecked(false);
    ui_->pushButton_firmware->setChecked(false);
    ui_->pushButton_testing->setChecked(false);
    ui_->pushButton_tuning->setChecked(false);
    ui_->pushButton_get_help->setChecked(false);
    ui_->pushButton_home->setChecked(true);
}

int PortConnection::GetCurrentTab(){
    return ui_->stackedWidget->currentIndex();
}

void PortConnection::SetPortConnection(bool state) {
  if (state) {
    connection_state_ = 1;
        ui_->connect_button->setText("DISCONNECT");
  } else {
    connection_state_ = 0;
    ports_names_.clear();
    selected_port_name_.clear();

    ui_->serial_port_combo_box->clear();

    ui_->label_firmware_build_value->setText(QString(""));
    ui_->label_firmware_name->setText(QString(""));
    ui_->label_hardware_name->setText(QString(""));
    ui_->label_bootloader_value->setText(QString(""));
    ui_->label_upgrader_value->setText(QString(""));
    
    ui_->connect_button->setText("CONNECT");

    AddToLog("module disconnected");
  }
}

void PortConnection::ConnectMotor(){

  int firmware_valid = 0;

  //Check if the firmware is valid
  firmware_valid = GetFirmwareValid();

  SetPortConnection(1);

  //We've succesfully connected at this point, so let's make sure the log is at most our maximum lines
  uint32_t lines_in_log = GetLinesInLog();

  if(lines_in_log > MAXIMUM_LINES_IN_LOG_FILE){
    ShortenLog(lines_in_log);
  }

  QString message = "Module Connected with Module ID: " + ui_->selected_module_combo_box->currentText();

  //Write the fact that we connected with the motor to the output log
  AddToLog("\n");
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
}

void PortConnection::UpdateGuiWithModuleIds(uint8_t module_id_with_different_sys_control){
    //Make sure to update our "detected modules" box
    QString discovered_string = QString::number(num_modules_discovered_);
    ui_->detected_modules_value->setText(discovered_string);

    QString message = "Discovered " + discovered_string + " module IDs";
    AddToLog("\n");
    AddToLog(message);
    ui_->header_error_label->setText(message);

    //Update our combo box
    ui_->selected_module_combo_box->clear();

    for(uint8_t i = 0; i < num_modules_discovered_; i++){

        //If we found a module ID of 0, and the parameter input is not 99, then we know
        //we have an old module on the bus. We need to set its label to the input, but its value to 0
        QString module_id_str;

        if((detected_module_ids_[i] == 0) && (module_id_with_different_sys_control != NO_OLD_MODULES_FOUND)){
            module_id_str = QString::number(module_id_with_different_sys_control);
        }else{
            module_id_str = QString::number(detected_module_ids_[i]);
        }

      ui_->selected_module_combo_box->addItem(module_id_str, detected_module_ids_[i]);
      AddToLog("Found module with Module ID: " + module_id_str);
    }

    // Center align items in selected_module_combo_box
    for(int i = 0; i < ui_->selected_module_combo_box->count(); i++){
      ui_->selected_module_combo_box->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    }
}

void PortConnection::DetectModulesClickedCallback(){
    //If someone clicked detect, and the serial port is actually connected,
    //then call DetectNumberOfModulesOnBus
    if(connection_state_ == 1){
        DetectNumberOfModulesOnBus();
    }else{
        QString error = "Could not detect modules. No serial port is connected";
        AddToLog(error);
        ui_->header_error_label->setText(error);
    }
}

void PortConnection::DetectNumberOfModulesOnBus(){
  uint8_t temp_obj_id = 0;
  uint8_t module_id_with_system_control_zero = NO_OLD_MODULES_FOUND;

  bool found_in_bootloader = false;

  AddToLog("\n");
  AddToLog("Detecting modules on the bus");

  //Before we try to connect with iquart, let's check if anyone in the ST bootloader (recovery mode)
  //If we get a response to this, then we know that uniquely 1 module is in recovery mode.
  //We should go through and detect anyone we can who is not in recovery so people can better
  //know who they're recovering
  found_in_bootloader = CheckIfInBootLoader();

  //Only try to talk to the modules if we're connected to the PC serial
  if(ser_.ser_port_->isOpen()){

      QString message = "Detecting Modules . . . ";
      ui_->header_error_label->setText(message);
      ui_->header_error_label->repaint();

      //Ensure that we're starting from scratch with our detections
      ClearDetections();

      //Go through each of the possible module IDs, and see if we get a response from its system control client
      for(uint8_t object_id = 0; object_id < MAX_MODULE_ID; object_id++){
        //Update the system control object id so that we are actually targeting different module ids
        sys_map_["system_control_client"]->UpdateClientObjId((object_id));

        //There's a case here in which someone hasn't updated their firmware, and they have a
        //Module id that is not 0, but a system control ID that is 0.
        //Note that if we get a response to 0, there are two possiblities
            // 1. There is one unique module with Module ID = 0
            // 2. There is one unique module with Module ID != 0, but system control obj id = 0

        //If that is the case, then we need to know in order to deal with them properly
        //If we get a response, then store that module id in the array, and increment our discovered modules value
        if(GetEntryReply(ser_, sys_map_["system_control_client"], "module_id", 2, 0.01f, temp_obj_id)){
            //We got a response on 0, but the value gotten isn't 0 (the case descibed above)
            if((object_id == 0) && (temp_obj_id != 0)){
                //We'll have to add it to our known modules as 0 in order to connect with it properly
                module_id_with_system_control_zero = temp_obj_id;
                detected_module_ids_[num_modules_discovered_] = 0;
            }else{
                detected_module_ids_[num_modules_discovered_] = temp_obj_id;
            }

          num_modules_discovered_++;
        }//if we get a response to this object id

      }//for()

      //Update our gui and log with who we've found
      UpdateGuiWithModuleIds(module_id_with_system_control_zero);

      if(found_in_bootloader){
        //Every second, the TimerTimeout function gets called.
        //in the case that we are re-detecting to deal with a recovery module later
        //we need to say that connection_state_ is false until we've dealt with the popup
        //If we don't, then the timer will cause us to try and talk to another module
        //which it can't...which will cause an attempt to reconnect, which will pop up a second
        //recovery message.
        bool temp_connection = connection_state_;
        connection_state_ = 0;

        //If we're trying to recover right now, then we should stop here!
        if(DisplayRecoveryMessage()){
            connection_state_ = temp_connection;
            return;
        }
      }

      if(num_modules_discovered_ > 0){
        //Update system control to the value stored in detected_module_ids_. This may be 0
        //if you are using old firmware with a non-0 module id
        sys_map_["system_control_client"]->UpdateClientObjId(detected_module_ids_[0]);

        //The buzzer gets the actual module ID, not necessarily the same value that is stored in detected_module_ids_.
        //With old firmware the stored value would be 0 (for system control 0), but we actually want to set
        //buzzer's ID to the stored module ID which is in the combo box
        indication_handle_.UpdateBuzzerObjId(ui_->selected_module_combo_box->currentText().toUInt());

        //We've found the modules, connect to the lowest module id
        ConnectMotor();

      }else{
        //we didn't find anything, so reset to default, and close the port
        sys_map_["system_control_client"]->UpdateClientObjId(DEFAULT_OBJECT_ID);
        indication_handle_.UpdateBuzzerObjId(DEFAULT_OBJECT_ID);
        ser_.ser_port_->close();
      }

  }else{
      AddToLog("Could not detect modules. No serial port open.");
  }
}

void PortConnection::ModuleIdComboBoxIndexChanged(int index){
    uint8_t obj_id = detected_module_ids_[index];
    sys_map_["system_control_client"]->UpdateClientObjId(obj_id);

    //The buzzer gets the actual module ID, not necessarily the same value that is stored in detected_module_ids_.
    //With old firmware the stored value would be 0 (for system control 0), but we actually want to set
    //buzzer's ID to the stored module ID which is in the combo box
    indication_handle_.UpdateBuzzerObjId(ui_->selected_module_combo_box->currentText().toUInt());

    uint8_t temp_id = 0;
    //Let's check that it's there. if not, don't try to connect
    if(GetEntryReply(ser_, sys_map_["system_control_client"], "module_id", 2, 0.01f, temp_id)){
        ConnectMotor();
    }

    //if we don't connect, then all we'll try to do is send the heartbeat to the new module id
    //when we don't get a response to that, we'll flash the "couldn't find that motor anymore" message
    //and then auto find the next available
}

void PortConnection::ConnectToSerialPort() {
  if (connection_state_ == 0) {
    if (!selected_port_name_.isEmpty()) {
      QString message = "Detecting Modules . . . ";
      ui_->header_error_label->setText(message);
      ui_->header_error_label->repaint();

      connection_state_ = 0;

      ser_.InitSerial(selected_port_name_, selected_baudrate_);
      try {

        //Try to open the serial port in general
        if (!ser_.ser_port_->open(QIODevice::ReadWrite)){
          throw QString("CONNECTION ERROR: could not open serial port");
        }

        AddToLog("Successfully opened serial port: " + selected_port_name_);

        //We were able to connect to the serial port, now let's try and find some modules
        DetectNumberOfModulesOnBus();

      } catch (const QString &e) {
        ClearDetections();
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
    ClearDetections();
    ClearFirmwareChoices();
    delete ser_.ser_port_;
    SetPortConnection(0);
    emit LostConnection();
  }
}

void PortConnection::ClearDetections(){
    //Reset our vars
    num_modules_discovered_ = 0;

    //Reset our GUI
    ui_->detected_modules_value->setText(QString::number(0));
    ui_->selected_module_combo_box->clear();
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

QString PortConnection::GetHardwareNameFromResources(int hardware_type){

    MetadataHandler temp_metadata_handler;

    /**
     * We have access to the Port Connection's electronics type and hardware type, but we do not have access to
     * the tab_populator's version of the resource files. So, we need to grab them ourself
     */
    //Only try to do this if the hardware type is a real value, and not -1 thrown in recovery
    if(hardware_type >= 0){
        QString current_path = QCoreApplication::applicationDirPath();
        QString hardware_type_file_path =
            current_path + "/Resources/Firmware/" + QString::number(hardware_type) + ".json";

        /**
         * Get the hardware type from the resource files. This is the
         * type of motor people should download for. Hardware name
         * is something like "vertiq 8018 150Kv
         */
        QString hardwareName = temp_metadata_handler.ArrayFromJson(hardware_type_file_path).at(0).toObject().value("hardware_name").toString();

        return hardwareName;
    }

    return "";
}

bool PortConnection::DisplayRecoveryMessage(){

    recovery_port_name_ = selected_port_name_;

    QMessageBox msgBox;
    msgBox.setWindowTitle("Recovery Mode Recognized");
    msgBox.setText(
        "It appears that a connected module is in Recovery Mode. If you would like to recover this module now, "
        "please select Recover Now. Otherwise, select Continue. You will be unable to communicate with "
        "the module in recovery mode until it is recovered. To recover the module after selecting Continue,"
        " please re-run DETECT, and select Recover Now.");

    QAbstractButton * continueButton = msgBox.addButton("Continue", QMessageBox::YesRole);
    QAbstractButton * recoverButton = msgBox.addButton("Recover Now", QMessageBox::NoRole);

    msgBox.exec();

    if (msgBox.clickedButton() == recoverButton) {
      //If we want to recover now, then close the port. Port gets handled in the recovery process
      //We don't want to close the port now if we're ignoring the recovery guy
      ser_.ser_port_->close();

      //kill our knowledge of anything else on the bus
      ClearDetections();

      //Don't let people leave
      DisableAllButtons();

      //Check to see if we know the type of motor from a previous connection in this session
      //If we don't know what the last motor was directly, then we need to go grab it from the log
      if(hardware_type_ == -1 && electronics_type_ == -1){
          //Ok...so we are trying to connect to a motor that can't talk to us. Let's use the "cache" method in which we assume that the most
          //recent connection is the same type of module as they're trying to recover. This is a value we can steal from the persistent log!
          FindHardwareAndElectronicsFromLog(&previous_handled_connection.hardware_value, &previous_handled_connection.electronics_value);
      }else{

          //hardware_type_ and electronics_type_ are values that get filled in during module connection
          previous_handled_connection.hardware_value = hardware_type_;
          previous_handled_connection.electronics_value = electronics_type_;
      }

      //regardless of where we got the previous connection from, we have it at this point. We can now
      //grab the hardware name from the hardware value
      QString previously_connected_module = GetHardwareNameFromResources(previous_handled_connection.hardware_value);

      //Pop up our message and determine if our guess is correct about what module is connected
      HandleFindingCorrectMotorToRecover(previously_connected_module);

      ui_->stackedWidget->setCurrentIndex(6);
      AddToLog("motor connected in recovery mode\n");

      return true;

    }else{
      ui_->pushButton_home->setChecked(true);
      ui_->stackedWidget->setCurrentIndex(0);

      return false;
    }

    return false;
}

void PortConnection::HandleFindingCorrectMotorToRecover(QString detected_module){
    //Give the user the option to reboot the module after setting with defaults.
    QMessageBox msgBox;
    QAbstractButton * correctButton = msgBox.addButton("Correct", QMessageBox::YesRole);
    QAbstractButton * wrongButton = msgBox.addButton("Recover a Different Module", QMessageBox::NoRole);

    msgBox.setWindowTitle("Select Recovery Module");

    QString text;

    //we couldn't find the module at all
    if(detected_module == ""){
        //if we don't know your module type, don't give the option to click correct
        text = "We cannot determine your module type while in recovery mode, and could not determine any previously connected modules.";
        msgBox.removeButton(correctButton);
        wrongButton->setText("Ok");
    }else{
        text = "We cannot determine your module type while in recovery mode, but you've recently connected to a " + detected_module +
                       ". Is this the module you would like to recover?";
    }

    msgBox.setText(text);
    msgBox.exec();

    AddToLog(text);

    //We only care if they click wrong. if it's wrong we'll have to help them find the correct
    //module to recover another way (aka...when they pick a file...make absolutely sure it's what they want)
    if(msgBox.clickedButton() == wrongButton){

        guessed_module_type_correctly_ = false;

        text = "Please use care to ensure that you download and program the correct firmware onto your module. Failure to do so "
               "could damage your module.";

        //Reuse the box that we already have, just change the text in it as well as the buttons
        msgBox.setText(text);
        wrongButton->setText("Ok");
        msgBox.removeButton(correctButton);

        msgBox.exec();

        AddToLog("Assumed module type incorrectly. Cannot verify the type of module connected.");
    }else{
        //If we have the right module, we don't need to do anything besides this var
        guessed_module_type_correctly_ = true;
        AddToLog("Assumed module type correctly as: " + detected_module);
    }
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

    //At this point we've gotten told what the module's ID is, store it in obj_id_. If this is different than our sys_map_ obj_id, we need to make
    //sure to use 0 as our system control obj_id instead of obj_id_ we filled here

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

    //add these to the log so that it is easer to protect people from bricking their motor in recovery mode
    //We can parse the log to see the most recent connection if we don't have the value saved (which we don't
    //if someone opens the control center fresh and tries to connect to a recovery mode module)
    AddToLog(hardware_str_ + QString::number(hardware_type));
    AddToLog(electronics_str_ + QString::number(electronics_type));

    //Make sure to store who we're connected to in our struct
    previous_handled_connection.hardware_value = hardware_type_;
    previous_handled_connection.electronics_value = electronics_type_;
}

int PortConnection::GetFirmwareValid(){
    int firmware_valid;
    //Check to see if the firmware is valid
    if(!GetEntryReply(ser_, sys_map_["system_control_client"], "firmware_valid", 5, 0.05f,
                      firmware_valid)){

      QString error_str("FIRMWARE ERROR: unable to determine firmware validity. Is your module powered on?");
      AddToLog(error_str);
      throw(error_str);
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
  if (connection_state_ == 1 && perform_timer_callback_) {
    uint8_t obj_id;
    //we didn't get a reply from our target module
    if (!GetEntryReply(ser_, sys_map_["system_control_client"], "module_id", 5, 0.05f, obj_id)) {
        //If we have multiple modules on the bus, we should rescan, and connect to a new module...and let the users know what's going on
        //if there's no one left, then we should just give up and close the port
        if(num_modules_discovered_ > 1){

            //pop up a message for them telling what just happened
            QMessageBox msgBox;
            msgBox.setWindowTitle("Module Disconnected");
            msgBox.setText(
                "Your currently targeted module has disconnected. We will rescan the network, and reconnect to another module.");
            msgBox.exec();

            //Find who's here now
            DetectNumberOfModulesOnBus();

        }else{
          ClearDetections();
          delete ser_.ser_port_;
          SetPortConnection(0);
          QString error_message = "Serial Port Disconnected";
          ui_->header_error_label->setText(error_message);
          emit LostConnection();
        }
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
  ui_->serial_port_combo_box->clear();
  Q_FOREACH (QSerialPortInfo port_info, QSerialPortInfo::availablePorts()) {
    ports_names_.push_back(port_info.portName());
    ui_->serial_port_combo_box->addItem(port_info.portName());
  }
  if (ports_names_.size() != 0) {
    PortComboBoxIndexChanged(0);
  }
}

void PortConnection::BaudrateComboBoxIndexChanged(int index) {
  selected_baudrate_ = ui_->serial_baud_rate_combo_box->currentData().value<int>();
}

void PortConnection::FindBaudrates() {
  ui_->serial_baud_rate_combo_box->clear();
  //QSerialPort only goes up to 115200, and its a back-end library, not part of our project, so best not to modify it. Add our own extra baud rate enum to port_connection to cover this and future cases
  ui_->serial_baud_rate_combo_box->addItem(QStringLiteral("921600"), Baud921600);
  ui_->serial_baud_rate_combo_box->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
  ui_->serial_baud_rate_combo_box->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
  ui_->serial_baud_rate_combo_box->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
  ui_->serial_baud_rate_combo_box->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
  selected_baudrate_ = ui_->serial_baud_rate_combo_box->currentData().value<int>();
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

void PortConnection::RebootMotor(){
    AddToLog("rebooting module\n");
    sys_map_["system_control_client"]->Set(ser_, std::string("reboot_program"));

    //Don't wait for anything. do this now!
    ser_.SendNow();
}

std::map<std::string, Client *> PortConnection::GetSystemControlMap(){
    return sys_map_;
}

uint8_t PortConnection::GetSysMapObjId(){
    return sys_map_["system_control_client"]->GetClientObjId();
}

bool PortConnection::ModuleIdAlreadyExists(uint8_t module_id){
    //findText goes through all of the labels stored in the combo box
    //Looking for labels rather than the data automatically checks the old firmware case
    //returns -1 if it's not there
    int already_exists = ui_->selected_module_combo_box->findText(QString::number(module_id));

    return already_exists > -1;
}

void PortConnection::PlayIndication(){
    indication_handle_.PlayIndication();
}

void PortConnection::HandleRestartNeeded(){
    bool should_redetect = false;

    //Don't let the timer timeout try and talk to us while we're doing this
    perform_timer_callback_ = false;

    RebootMotor();

    //Pop up a message saying what's going on
    //Give the user the option to reboot the module after setting with defaults.
    QMessageBox msgBox;
    msgBox.setWindowTitle("Reboot Required");

     QString text = "Setting this parameter requires a module reboot to take effect. We are rebooting your module now.";

     //If we have multiple modules on the bus, we should rescan, and connect to a new module...and let the users know what's going on
     //if there's no one left, then we should just give up and close the port
     if(num_modules_discovered_ > 1){

        //We've got others here, make sure to redetect
        should_redetect = true;

        //pop up a message for them telling what just happened
        text = text + " We will automatically rescan the network.";

     }

     msgBox.setText(text);
     msgBox.exec();

     if(should_redetect){
         //Stop here for 2 seconds to make sure everyone is rebooted!
         QTime end_pause = QTime::currentTime().addSecs(2);
         while (QTime::currentTime() < end_pause)
             QCoreApplication::processEvents(QEventLoop::AllEvents);

         //Find who's here now
         DetectNumberOfModulesOnBus();
     }

     //Let the timer callback work again
     perform_timer_callback_ = true;
}

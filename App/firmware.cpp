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

#include "firmware.h"

#include <JlCompress.h>

#include <QDebug>
#include <QFile>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

Firmware::Firmware(QProgressBar *flash_progress_bar, QPushButton *firmware_binary_button)
    : flash_progress_bar_(flash_progress_bar), firmware_binary_button_(firmware_binary_button) {
  sys_map_ = ClientsFromJson(0, "system_control_client.json", clients_folder_path_);
}

void Firmware::SelectFirmwareClicked() {
  try {
    iv.label_message->clear();
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);

    QString desktop_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    //Opens a selected file. It sets the dialog to have a drop down that allows you to search for a bin or zip.
    //Change the order of the tr() to change the default
    firmware_bin_path_ = QFileDialog::getOpenFileName(0, ("Select Firmware Binary or Archive"), desktop_dir,
                                                      tr("Zip (*.zip) ;; Binary (*.bin)"));

    //Check if it's empty. If it is do nothing and keep displaying the text
    //If we picked a bin file, process it as we always have
    //If we picked a zip folder, use quazip
    if (firmware_bin_path_.isEmpty()) {
      firmware_binary_button_->setText("Select Firmware (\".bin\") or (\".zip\")" );

    } else if(firmware_bin_path_.contains(".bin")){
        QFileInfo info(firmware_bin_path_);
        firmware_binary_button_->setText(info.fileName());

        //Pop up a warning window about dangers of flashing a binary

        QMessageBox msgBox;
        msgBox.setWindowTitle("WARNING!");
        msgBox.setText(
            "Flashing an incorrect binary can cause damage to your motor. Are you sure you want to continue?");

        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if(msgBox.exec() == QMessageBox::No){
            firmware_binary_button_->setText("Select Firmware (\".bin\") or (\".zip\")" );
            firmware_bin_path_ = "";
            return;
        }
    }else if(firmware_bin_path_.contains(".zip")){
        //extract the archive, then we can treat it normally as a folder
        JlCompress extract_tool;
        //Extract to wherever we're running the project right now
        extract_path_ = qApp->applicationDirPath() + "/flash_dir";
        extract_tool.extractDir(firmware_bin_path_, extract_path_);

        //This prints the full path to the folder. Might need to do some work to get it to
        //Match what info.fileName does now
        //Set the button text to the folder that the user put in
        firmware_binary_button_->setText(firmware_bin_path_);

        //In the background, use the extracted directory
        //For testing right now, just use the combined.bin.
        //Later on I need to figure out how to present the different options for how to flash
        firmware_bin_path_ = extract_path_ + "/combined.bin";
    }

  } catch (const QString &e) {
    iv.label_message->setText(e);
  }
}

QJsonArray Firmware::OpenResourceFile(){
//Read the resource file for the motor connected
    QString current_path = QCoreApplication::applicationDirPath();
    QString hardware_type_file_path =
        current_path + "/Resources/Firmware/" + QString::number(iv.pcon->GetHardwareType()) + ".json";

    //Read and close the json file
    QFile resourceJson;
    resourceJson.setFileName(hardware_type_file_path);
    //Grab all of the data from the json
    resourceJson.open(QIODevice::ReadOnly);
    QString resourceInfo = resourceJson.readAll();
    resourceJson.close();

    //Read the data stored in the json as a json document
    QJsonDocument resourceDoc = QJsonDocument::fromJson(resourceInfo.toUtf8());
    return resourceDoc.array();
}

QJsonArray Firmware::ArrayFromJson(QString pathToJson){
    //If we have a file ready to flash, we need to reference the json to make sure that the firmware we picked
    //Is meant for the hardware connected
    QFile jsonFile;
    jsonFile.setFileName(pathToJson); //It is called this for right now. It needs to have a name set somewhere else
                                                                   //So it's not hard coded
    //Grab all of the data from the json
    jsonFile.open(QIODevice::ReadOnly);
    QString val = jsonFile.readAll();
    jsonFile.close();

    //Read the data stored in the json as a json document
    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    //Our json stores each entry contained within an array
    return jsonDoc.array();
}

QString Firmware::GetHardwareTypeFromJson(QJsonArray array){
    return array.at(0).toObject().value("hardware_name").toString();
}

void Firmware::FlashClicked() {
  QString seletected_port_name = iv.pcon->GetSelectedPortName();

  if (firmware_bin_path_.isEmpty()) {
    QString err_message = "No Firmware Binary Selected";
    iv.label_message->setText(err_message);
    return;
  }

  //We only want to check on the json information if we know that there is a json to check. This only happens
  //if the extract_path_ variable isnt ""
  if(extract_path_ != ""){
      //Right now this is hard coded to new_json_mockup but must change to grab whatever the json is called!!
      QJsonArray expectedMotorInfo = ArrayFromJson(extract_path_ + "/new_json_mockup.json");

      //Grabbing data from the first entry (hardare and electronics type)
      QJsonObject safetyObj = expectedMotorInfo.at(0).toObject();
      int toFlashElectronicsType = safetyObj.value("to_flash_electronics_type").toInt();
      int toFlashHardwareType = safetyObj.value("to_flash_hardware_type").toInt();

      //If the value we are meant to flash does not match the current motor throw a warning and don't allow flashing
      //A wrong value could be a mismatched Kv or incorrect motor type
      if((toFlashElectronicsType != iv.pcon->GetElectronicsType()) || (toFlashHardwareType != iv.pcon->GetHardwareType())){

          QString current_path = QCoreApplication::applicationDirPath();
          QString hardware_type_file_path =
              current_path + "/Resources/Firmware/" + QString::number(iv.pcon->GetHardwareType()) + ".json";
          //Get the hardware type name from the resource files
          //This is the type of motor people should download for
          QString hardwareName = GetHardwareTypeFromJson(ArrayFromJson(hardware_type_file_path));

          QString errorType;
          if(toFlashElectronicsType != iv.pcon->GetElectronicsType()){
              errorType = "Firmware is for the wrong Electronics Type";
          }else{
              errorType = "Firmware is for the wrong Hardware Type";
          }

          QMessageBox msgBox;
          msgBox.setWindowTitle("WARNING!");
          msgBox.setText(
              "The firmware you are trying to flash is not meant for this motor. Please go to vertiq.co "
              "and download the correct file for your motor: " + hardwareName + "\n\n" + "Error: " + errorType);

          msgBox.setStandardButtons(QMessageBox::Ok);
          msgBox.setDefaultButton(QMessageBox::Ok);
          if(msgBox.exec() == QMessageBox::Ok){
              return;
        }
    }
}

  if (!BootMode()) {
    return;
  }

  try {
    Schmi::ErrorHandlerStd error;
    Schmi::BinaryFileStd bin(firmware_bin_path_.toStdString());
    Schmi::QSerial ser(seletected_port_name);
    FlashLoadingBar bar(flash_progress_bar_);

    Schmi::FlashLoader fl(&ser, &bin, &error, &bar);

    fl.Init();

    std::chrono::steady_clock::time_point time_start = std::chrono::steady_clock::now();
    bool boot_mode = false;
    while (!boot_mode) {
      boot_mode = fl.InitUsart();
      if (std::chrono::steady_clock::now() - time_start > std::chrono::milliseconds(10000)) {
        throw QString("Could Not Init UART From Boot Mode");
        break;
      };
    }

    bool init_usart = false;
    fl.Flash(init_usart);

    //We are now done with the extracted directory that we made. We should delete it to avoid any issues
    QDir dir(qApp->applicationDirPath() + "/flash_dir");
    dir.removeRecursively();

  } catch (const QString &e) {
    iv.label_message->setText(e);
  }
}

bool Firmware::BootMode() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      QSerialInterface *ser = iv.pcon->GetQSerialInterface();
      if (!sys_map_["system_control_client"]->Set(*ser, std::string("reboot_boot_loader")))
        throw QString("COULDN'T REBOOT: please reconnect or try again");
      ser->SendNow();

      iv.label_message->setText("Waiting for motor to go in BootMode");
      // If you delete the port too fast, the ftdi chip will die before bytes were sent.
      QTime dieTime = QTime::currentTime().addMSecs(500);
      while (QTime::currentTime() < dieTime) {
        //        QCoreApplication::processEvents();
      }

      // delete and unconnects port
      delete ser->ser_port_;
      iv.pcon->SetPortConnection(0);
      emit iv.pcon->LostConnection();
      QCoreApplication::processEvents();

    } catch (const QString &e) {
      iv.label_message->setText(e);
      return 0;
    }
  } else {
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.label_message->setText(error_message);
    return 0;
  }
  return 1;
}

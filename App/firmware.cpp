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

void Firmware::UpdateFlashButtons(){
    //We now have access to the firmware types that this package supports. Hide flash buttons that we don't support
    QStringList flashTypes = metadata_handler_->GetFlashTypes();
    QStringList binTypes = metadata_handler_->GetBinariesInFolder();

    //This is going to need to update as we change the names of the entries
    //The final name will probably be something like "vertiq8108_150kv_speed_app"
    //So we still need to just look for app, just adds an extra step of grabbing each string and looking for the substr
    //Talk to the motor and ask what sections it currently has.
    //If we don't get a response (0), only allow combined
    //If we get 1, only allow combined (only app is present)
    //If we get 5, allow app, boot, and combined (we don't have DroneCAN upgrade)
    //If we get 7, allow all
    uint8_t apps_present = iv.pcon->GetAppsPresent();
    //If you aren't currently connected to a motor, don't want to present any options at all. We stop you from trying to flash anyway
    //But it doesn't automatically update the options if you select the file and then connect
    bool boot_present = apps_present & BOOT_PRESENT_MASK;
    bool upgrade_present = apps_present & UPGRADE_PRESENT_MASK;
    bool app_present = apps_present & APP_PRESENT_MASK;

    bool displayApp = flashTypes.contains("app") && binTypes.contains("app.bin") && app_present;
    bool displayUpgrade = flashTypes.contains("upgrade") && binTypes.contains("upgrade.bin") && upgrade_present;
    bool displayBoot = flashTypes.contains("boot") && binTypes.contains("boot.bin") && boot_present;
    bool displayCombined = flashTypes.contains("combined") && binTypes.contains("combined.bin");

    if(displayApp){
        iv.pcon->GetMainWindowAccess()->flash_app_button->setVisible(true);
        int appIndex;
        //If we have an app and no upgrade the index of app in the json is different
        if(!displayUpgrade){
            appIndex = 2;
        }else{
            appIndex = 3;
        }
        QString appMajor = QString::number(metadata_handler_->GetTypesArray(appIndex)->GetMajor());
        QString appMinor = QString::number(metadata_handler_->GetTypesArray(appIndex)->GetMinor());
        QString appPatch = QString::number(metadata_handler_->GetTypesArray(appIndex)->GetPatch());
        iv.pcon->GetMainWindowAccess()->flash_app_button->setText("Flash App v" + appMajor + "." + appMinor + "." + appPatch);
    }
    if(displayUpgrade){
        iv.pcon->GetMainWindowAccess()->flash_upgrade_button->setVisible(true);
        QString upgradeMajor = QString::number(metadata_handler_->GetTypesArray(2)->GetMajor());
        QString upgradeMinor = QString::number(metadata_handler_->GetTypesArray(2)->GetMinor());
        QString upgradePatch = QString::number(metadata_handler_->GetTypesArray(2)->GetPatch());
        iv.pcon->GetMainWindowAccess()->flash_upgrade_button->setText("Flash Upgrade v" + upgradeMajor + "." + upgradeMinor + "." + upgradePatch);
    }
    if(displayCombined){
        iv.pcon->GetMainWindowAccess()->flash_button->setVisible(true);
        iv.pcon->GetMainWindowAccess()->flash_button->setText("Flash Combined");
    }
    if(displayBoot){
        iv.pcon->GetMainWindowAccess()->flash_boot_button->setVisible(true);
        QString bootMajor = QString::number(metadata_handler_->GetTypesArray(1)->GetMajor());
        QString bootMinor = QString::number(metadata_handler_->GetTypesArray(1)->GetMinor());
        QString bootPatch = QString::number(metadata_handler_->GetTypesArray(1)->GetPatch());
        iv.pcon->GetMainWindowAccess()->flash_boot_button->setText("Flash Boot v" + bootMajor + "." + bootMinor + "." + bootPatch);
    }
}

void Firmware::SelectFirmwareClicked() {
  try {
    if(iv.pcon->GetConnectionState() != false){
        iv.label_message->clear();
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::AnyFile);

        QString desktop_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

        //Opens a selected file. It sets the dialog to have a drop down that allows you to search for a bin or zip.
        //Change the order of the tr() to change the default
        firmware_bin_path_ = QFileDialog::getOpenFileName(0, ("Select Firmware Binary or Archive"), desktop_dir,
                                                          tr("Zip (*.zip) ;; Binary (*.bin)"));

        //Check if it's empty. If it is do nothing and keep displaying the text
        //If we picked a bin file, process it as we always have, but flash a warning
        //If we picked a zip folder, use quazip
        if (firmware_bin_path_.isEmpty()) {
          firmware_binary_button_->setText("Select Firmware (\".bin\") or (\".zip\")" );

        } else if(firmware_bin_path_.contains(".bin")){
            using_metadata_ = false;
            QFileInfo info(firmware_bin_path_);
            firmware_binary_button_->setText(info.fileName());

            //Pop up a warning window about dangers of flashing a binary
            QMessageBox msgBox;
            msgBox.setWindowTitle("WARNING!");
            msgBox.setText(
                "Flashing raw binary files is very dangerous. If you flash firmware "
                "that is not meant for your motor (wrong section (boot, application, upgrade), hardware, electronics, etc.), "
                "you risk seriously damaging or breaking your motor. "
                "Continue at your own risk.\nAre you positive you wish to continue?");

            msgBox.setStandardButtons(QMessageBox::Yes);
            msgBox.addButton(QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            //If you pick no, reset to the init state
            if(msgBox.exec() == QMessageBox::No){
                firmware_binary_button_->setText("Select Firmware (\".bin\") or (\".zip\")" );
                firmware_bin_path_ = "";
                return;
            }

            //If you want to move forward, make the flash button available
            iv.pcon->GetMainWindowAccess()->flash_button->setVisible(true);
            iv.pcon->GetMainWindowAccess()->flash_app_button->setVisible(false);
            iv.pcon->GetMainWindowAccess()->flash_upgrade_button->setVisible(false);
            iv.pcon->GetMainWindowAccess()->flash_boot_button->setVisible(false);

            iv.pcon->GetMainWindowAccess()->flash_button->setText("Flash");

        //If you want to use a zip dir
        }else if(firmware_bin_path_.contains(".zip")){
            using_metadata_ = true;
            metadata_handler_ = new MetadataHandler(iv.pcon);
            //extract the archive, then we can treat it normally as a folder
            metadata_handler_->ExtractMetadata(firmware_bin_path_);

            //This displays the full path to the folder. Might need to do some work to get it to
            //Match what info.fileName does now
            //Set the button text to the folder that the user put in
            firmware_binary_button_->setText(firmware_bin_path_);

            //If there isn't a metadata json, throw a warning message and stop them from moving forward
            if(metadata_handler_->GetMetadataJsonPath() == ""){
                //Pop up a warning window about dangers of flashing a binary
                QMessageBox msgBox;
                msgBox.setWindowTitle("File Error!");
                msgBox.setText(
                    "It appears you are trying to use an archive not provided from Vertiq. "
                    "Please go to vertiq.co and redownload the correct archive for your motor.");

                msgBox.setStandardButtons(QMessageBox::Ok);
                //If you pick no, reset to the init state
                if(msgBox.exec() == QMessageBox::Ok){
                    firmware_binary_button_->setText("Select Firmware (\".bin\") or (\".zip\")" );
                    firmware_bin_path_ = "";
                    metadata_handler_->Reset(iv.pcon->GetMainWindowAccess());
                    return;
                }
            }

            //We are only ever going to have one json per released zip
            //Use that to find the one to grab the data from
            metadata_handler_->ReadMetadata();

            UpdateFlashButtons();
        }
    }else{
        QString error_message = "No Motor Connected, Please Connect Motor Before Selecting Firmware";
        iv.label_message->setText(error_message);
    }
  } catch (const QString &e) {
    iv.label_message->setText(e);
  }
}

bool Firmware::CheckPathAndConnection(){
    //Don't move forward if there is there's no binary selected or no motor connected
    if (firmware_bin_path_.isEmpty()) {
      QString err_message = "No Firmware Binary Selected";
      iv.label_message->setText(err_message);
      return true;
    }else if(iv.pcon->GetConnectionState() != 1){
        QString error_message = "No Motor Connected, Please Connect Motor";
        iv.label_message->setText(error_message);
        return true;
    }

    return false;
}

bool Firmware::FlashHardwareElectronicsWarning(){
    //If the value we are meant to flash does not match the current motor throw a warning and don't allow flashing
    //A wrong value could be a mismatched Kv or incorrect motor type
    if(!(metadata_handler_->CheckHardwareAndElectronics())){

        //We have access to the Port Connection's electronics type and hardware type, but we do not have access to
        //the tab_populator's version of the resource files. So, we need to grab them ourself
        QString current_path = QCoreApplication::applicationDirPath();
        QString hardware_type_file_path =
            current_path + "/Resources/Firmware/" + QString::number(iv.pcon->GetHardwareType()) + ".json";
        //Get the hardware type name from the resource files
        //This is the type of motor people should download for
        //Hardware name is something like "vertiq 8108 150Kv"
        QString hardwareName = metadata_handler_->ArrayFromJson(hardware_type_file_path).at(0).toObject().value("hardware_name").toString();

        //Determine which thing they have wrong
        QString errorType;
        errorType = metadata_handler_->GetErrorType();

        QMessageBox msgBox;
        msgBox.setWindowTitle("WARNING!");
        msgBox.setText(
            "The firmware you are trying to flash is not meant for this motor. Please go to vertiq.co "
            "and download the correct file for your motor: " + hardwareName + "\n\n" + "Error(s): " + errorType);

        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        if(msgBox.exec() == QMessageBox::Ok){
            firmware_binary_button_->setText("Select Firmware (\".bin\") or (\".zip\")" );
            firmware_bin_path_ = "";
            metadata_handler_->Reset(iv.pcon->GetMainWindowAccess());
            return true;
        }
    }

    return false;
}

void Firmware::FlashCombinedClicked(){
    type_flash_requested_ = "combined";
    if(using_metadata_){
        iv.pcon->SaveNewBootloaderVersion(metadata_handler_->GetBootloaderVersion());
        iv.pcon->SaveNewUpgraderVersion(metadata_handler_->GetUpgradeVersion());
    }
    FlashClicked();
}

void Firmware::FlashBootClicked() {
    type_flash_requested_= "boot";

    //Save the new boot version to the motor
    iv.pcon->SaveNewBootloaderVersion(metadata_handler_->GetBootloaderVersion());
    FlashClicked();
}

void Firmware::FlashAppClicked(){
    type_flash_requested_ = "app";
    FlashClicked();
}

void Firmware::FlashUpgradeClicked() {
    type_flash_requested_= "upgrade";

    iv.pcon->SaveNewUpgraderVersion(metadata_handler_->GetUpgradeVersion());
    FlashClicked();
}

void Firmware::FlashClicked() {

    //After you click Flash Combined, set the binary path to the combined binary
    //Check that you have a good file and are connected to a motor
    //Check that the hardware and electronics are correct
    //Flash the motor
    uint32_t startingMemoryLocation = DEFAULT_STARTING_LOCATION_;

    //Only do this check if actaully have metadata to look at
    if(using_metadata_){
        //Save the new boot version to the motor
       firmware_bin_path_ = metadata_handler_->GetPathToCorrectBin(type_flash_requested_);
       if(FlashHardwareElectronicsWarning()){
           return;
       }

       startingMemoryLocation = metadata_handler_->GetStartingMemoryFromType(type_flash_requested_);

    }

    if(CheckPathAndConnection()){
        return;
    }

    FlashFirmware(startingMemoryLocation);
}

void Firmware::FlashFirmware(uint32_t startingPoint){
    QString seletected_port_name = iv.pcon->GetSelectedPortName();

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
      bool global_erase = false;
      fl.Flash(init_usart, global_erase, startingPoint);

      //We are now done with the extracted directory that we made. We should delete it to avoid any issues
      if(using_metadata_){
        metadata_handler_->Reset(iv.pcon->GetMainWindowAccess());
      }

    } catch (const QString &e) {
        iv.label_message->setText(e);
    }
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

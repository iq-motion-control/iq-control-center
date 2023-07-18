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

Firmware::Firmware(){}

Firmware::Firmware(QProgressBar *flash_progress_bar, QPushButton *firmware_binary_button, QProgressBar *recover_progress_bar, QPushButton *recover_binary_button){
    Init(flash_progress_bar, firmware_binary_button, recover_progress_bar, recover_binary_button);
}

void Firmware::Init(QProgressBar *flash_progress_bar, QPushButton *firmware_binary_button, QProgressBar *recover_progress_bar, QPushButton *recover_binary_button){
    flash_progress_bar_ = flash_progress_bar;
    firmware_binary_button_ = firmware_binary_button;
    recover_progress_bar_ = recover_progress_bar;
    recover_binary_button_ = recover_binary_button;
    sys_map_ = ClientsFromJson(0, "system_control_client.json", clients_folder_path_, nullptr, nullptr);

    //If the Control Center closed or crashed without completing a flash, there may be leftover local metadata files.
    //Clearing them out on startup so we start with a fresh slate. Make a temporary metadata handler just to clear out any old files.
    MetadataHandler * temporary_startup_metadata_handler = new MetadataHandler(iv.pcon);
    temporary_startup_metadata_handler->DeleteExtractedFolder();
    delete temporary_startup_metadata_handler;
}

void Firmware::UpdateFlashButtons(){
    //We now have access to the firmware types that this package supports. Hide flash buttons that we don't support
    QStringList flashTypes = metadata_handler_.GetFlashTypes();
    QStringList binTypes = metadata_handler_.GetBinariesInFolder();

    /**
     * This is going to need to update as we change the names of the entries
     * in time, consider upgrading to a name format such as: "vertiq8108_150kv_speed_app"
     * So we still need to just look for app, just adds an extra step of grabbing each string and looking for the substr
    */

    /**
     * Talk to the motor and ask what sections it currently has.
     * Depending on the current state of the motor, only display certain
     * flash options with the buttons
     */
    uint8_t apps_present = iv.pcon->GetAppsPresent();

    //If you aren't currently connected to a motor, don't want to present any options at all. We stop you from trying to flash anyway
    //But it doesn't automatically update the options if you select the file and then connect
    boot_present_ = apps_present & BOOT_PRESENT_MASK;
    upgrade_present_ = apps_present & UPGRADE_PRESENT_MASK;
    app_present_ = apps_present & APP_PRESENT_MASK;

    //Logic to determine which flash options to present
    //flashTypes holds the information about what sections are allowed to be flashed according to the json metadata
    //binTypes holds the information about what binary files are contained in the zip

    //App should only show up as an option if there is both an app and boot on the motor (means it's partitioned)
    bool displayApp = flashTypes.contains("app") && binTypes.contains("app.bin") && app_present_ && boot_present_;

    //Upgrade should only appear if there is an upgrade already there to upgrade
    bool displayUpgrade = flashTypes.contains("upgrade") && binTypes.contains("upgrade.bin") && upgrade_present_;

    //Boot should only appear if there is a boot already there to upgrade
    bool displayBoot = flashTypes.contains("boot") && binTypes.contains("boot.bin") && boot_present_;

    //Always unless there is a serious problem with our release, or we decide that we want to release separate binaries in differenet zips
    //This system gives us the flexability to release whatever we want in the zip as long as it has a json to go along with it
    bool displayCombined = (flashTypes.contains("combined") || flashTypes.contains("main")) &&
                           (binTypes.contains("combined.bin") || binTypes.contains("main.bin"));

    /**
     * Depending on the results of the logic above, we choose which buttons to make available to the user
     * Each button (excluding combined) gets its version as well as type for what it will flash
     */
    if(displayApp){
        iv.pcon->GetMainWindowAccess()->flash_app_button->setVisible(true);
        //If we have an app and no upgrade the index of app in the json is different
        if(!flashTypes.contains("upgrade")){
            app_index_ = 2;
        }else{
            app_index_ = 3;
        }
        QString appMajor = QString::number(metadata_handler_.GetTypesArray(app_index_).GetMajor());
        QString appMinor = QString::number(metadata_handler_.GetTypesArray(app_index_).GetMinor());
        QString appPatch = QString::number(metadata_handler_.GetTypesArray(app_index_).GetPatch());
        iv.pcon->GetMainWindowAccess()->flash_app_button->setText("Flash App v" + appMajor + "." + appMinor + "." + appPatch);
    }

    if(displayUpgrade){
        iv.pcon->GetMainWindowAccess()->flash_upgrade_button->setVisible(true);
        QString upgradeMajor = QString::number(metadata_handler_.GetTypesArray(UPGRADE_INDEX).GetMajor());
        QString upgradeMinor = QString::number(metadata_handler_.GetTypesArray(UPGRADE_INDEX).GetMinor());
        QString upgradePatch = QString::number(metadata_handler_.GetTypesArray(UPGRADE_INDEX).GetPatch());
        QString upgradeStyle = QString::number(metadata_handler_.GetTypesArray(UPGRADE_INDEX).GetStyle());
        iv.pcon->GetMainWindowAccess()->flash_upgrade_button->setText("Flash Upgrade v" + upgradeStyle + "."+ upgradeMajor + "." + upgradeMinor + "." + upgradePatch);
    }

    if(displayBoot){
        iv.pcon->GetMainWindowAccess()->flash_boot_button->setVisible(true);
        QString bootStyle = QString::number(metadata_handler_.GetTypesArray(BOOT_INDEX).GetStyle());
        QString bootMajor = QString::number(metadata_handler_.GetTypesArray(BOOT_INDEX).GetMajor());
        QString bootMinor = QString::number(metadata_handler_.GetTypesArray(BOOT_INDEX).GetMinor());
        QString bootPatch = QString::number(metadata_handler_.GetTypesArray(BOOT_INDEX).GetPatch());
        iv.pcon->GetMainWindowAccess()->flash_boot_button->setText("Flash Boot v" + bootStyle + "." + bootMajor + "." + bootMinor + "." + bootPatch);
    }

    if(displayCombined){
        iv.pcon->GetMainWindowAccess()->flash_button->setVisible(true);
        if(flashTypes.contains("main")){
            iv.pcon->GetMainWindowAccess()->flash_button->setText("Flash");
        }else{
            iv.pcon->GetMainWindowAccess()->flash_button->setText("Flash Combined");
        }
    }

}

void Firmware::SelectFirmwareClicked() {

  int currentTab = iv.pcon->GetCurrentTab();
  try {

    /**
     * When a motor enters recovery mode, it loses connection with the control center. Therefore, without this
     * additional logic, a motor in recovery mode wouldn't be allowed to select a recovery file.
     * This stops you from being able to choose a file unless you are connected to a motor or are in recovery. This
     * helps us avoid some weird UI things.
     */
    if(iv.pcon->GetConnectionState() != false || currentTab == RECOVERY_TAB){
        iv.label_message->clear();
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::ExistingFile);

        QPushButton * buttonInUse;

        QString desktop_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

        //Opens a selected file. It sets the dialog to have a drop down that allows you to search for a bin or zip.
        //Change the order of the tr() to change the default
        firmware_bin_path_ = QFileDialog::getOpenFileName(0, ("Select Firmware Binary or Archive"), desktop_dir,
                                                          tr("Binary (*.bin) ;; Zip (*.zip)"));

        iv.pcon->AddToLog("Firmware file selected: " + firmware_bin_path_);

        //Pick which button we want to use
        if(currentTab == FIRMWARE_TAB){
            buttonInUse = firmware_binary_button_;
        }else if(currentTab == RECOVERY_TAB){
            buttonInUse = recover_binary_button_;
        }

        //Make sure we clear out any old metadata before we try to unpack any new metadata.
        //The functionality for looking at the metadata and the binaries assumes there is only
        //one set of metadata and binaries active at any one time, but if the user switches selections
        //without flashing we could have lingering metadata
        ResetMetadata();

        //Check if it's empty. If it is do nothing and keep displaying the text
        //If we picked a bin file, process it as we always have, but flash a warning
        //If we picked a zip folder, use quazip
        if (firmware_bin_path_.isEmpty()) {
          buttonInUse->setText("Select Firmware (\".bin\") or (\".zip\")" );
        } else if(firmware_bin_path_.contains(".bin")){
            HandleDisplayWhenBinSelected(buttonInUse);
        //If you want to use a zip dir
        }else if(firmware_bin_path_.contains(".zip")){
            HandleDisplayWhenZipSelected(buttonInUse, currentTab);
        }

    }else{
        QString error_message = "No Motor Connected, Please Connect Motor Before Selecting Firmware";
        iv.label_message->setText(error_message);
        iv.pcon->AddToLog(error_message);
    }

  } catch (const QString &e) {
    iv.label_message->setText(e);
  }
}

void Firmware::HandleDisplayWhenZipSelected(QPushButton * buttonInUse, int currentTab){

    using_metadata_ = true;
    metadata_handler_.Init(iv.pcon);
    //extract the archive, then we can treat it normally as a folder
    metadata_handler_.ExtractMetadata(firmware_bin_path_);

    //Set the button text to the folder that the user put in
    buttonInUse->setText(firmware_bin_path_);

    //If there isn't a metadata json, throw a warning message and stop them from moving forward
    if(metadata_handler_.GetMetadataJsonPath() == ""){
        //Pop up a warning window about dangers of flashing a binary
        QMessageBox msgBox;
        msgBox.setWindowTitle("File Error!");
        msgBox.setText(
            "It appears you are trying to use an archive not provided from Vertiq. "
            "Please go to vertiq.co and redownload the correct archive for your motor.");

        iv.pcon->AddToLog("No json found in zip\n");

        msgBox.setStandardButtons(QMessageBox::Ok);
        //If you pick no, reset to the init state
        if(msgBox.exec() == QMessageBox::Ok){
            buttonInUse->setText("Select Firmware (\".bin\") or (\".zip\")" );
            firmware_bin_path_ = "";
            ResetMetadata();
            return;
        }
    }

    //We are only ever going to have one json per released zip
    //Use that to find the one to grab the data from
    metadata_handler_.ReadMetadata();

    //If the wrong type of motor is connect for the selected file, don't let them move forward
    //When we have gotten to this point, we've either confirmed that our guess at the correct type of module
    //was a correct guess, or the user has been told to use the dropdown on the recovery tab
    if(FlashHardwareElectronicsWarning(currentTab)){
        return;
    }


    //If we aren't in recovery, present only the options that can be safely flashed.
    //Only giving combined option from recovery mode
    if(currentTab == FIRMWARE_TAB){
        UpdateFlashButtons();
    }
}
void Firmware::HandleDisplayWhenBinSelected(QPushButton *buttonInUse){

    using_metadata_ = false;
    QFileInfo info(firmware_bin_path_);
    buttonInUse->setText(info.fileName());

//    //Pop up a warning window about dangers of flashing a binary
//    QMessageBox msgBox;
//    msgBox.setWindowTitle("WARNING!");
//    msgBox.setText(
//        "As flashing raw binaries can be dangerous, we strongly advise using a provided archive if available. If you flash firmware "
//        "that is not meant for your motor (wrong section (boot, application, upgrade), hardware, electronics, etc.), "
//        "you risk seriously damaging or breaking your motor."
//        "\nAre you sure you would like to continue?");

//    msgBox.setStandardButtons(QMessageBox::Yes);
//    msgBox.addButton(QMessageBox::No);
//    msgBox.setDefaultButton(QMessageBox::No);
//    //If you pick no, reset to the init state
//    if(msgBox.exec() == QMessageBox::No){
//        buttonInUse->setText("Select Firmware (\".bin\") or (\".zip\")" );
//        firmware_bin_path_ = "";
//        iv.pcon->GetMainWindowAccess()->flash_button->setVisible(false);
//        return;
//    }

    //If you want to move forward, make the flash button available
    iv.pcon->GetMainWindowAccess()->flash_button->setVisible(true);
    iv.pcon->GetMainWindowAccess()->flash_app_button->setVisible(false);
    iv.pcon->GetMainWindowAccess()->flash_upgrade_button->setVisible(false);
    iv.pcon->GetMainWindowAccess()->flash_boot_button->setVisible(false);

    iv.pcon->GetMainWindowAccess()->flash_button->setText("Flash");

}

bool Firmware::CheckPathAndConnection(){
    int curTab = iv.pcon->GetCurrentTab();
    //Don't move forward if there is there's no binary selected or no motor connected
    if (firmware_bin_path_.isEmpty()) {
      QString err_message = "No Firmware Binary Selected";
      iv.label_message->setText(err_message);

      iv.pcon->AddToLog("Failed to load binary: " + err_message);
      return true;
      //Motor is disconnected when we reach this point of a recovery
    }else if(iv.pcon->GetConnectionState() != 1 && curTab != RECOVERY_TAB){
        QString error_message = "No Motor Connected, Please Connect Motor";
        iv.label_message->setText(error_message);

        iv.pcon->AddToLog("Failed to load binary: " + error_message);
        return true;
    }

    return false;
}

bool Firmware::FlashHardwareElectronicsWarning(int current_tab){
    int target_hardware = -1;
    int target_electronics = -1;

    //We've already set previous_handled_connection in port connection to either
    //the last module connected during this session, or the last connected module
    //as read through the log
    if(current_tab == RECOVERY_TAB){
        if(iv.pcon->guessed_module_type_correctly_){
            target_hardware = iv.pcon->previous_handled_connection.hardware_value;
            target_electronics = iv.pcon->previous_handled_connection.electronics_value;
        }
    }else{
        target_hardware = iv.pcon->previous_handled_connection.hardware_value;
        target_electronics = iv.pcon->previous_handled_connection.electronics_value;
    }

    bool should_put_hardware_electronics_in_msg = (target_hardware != -1) && (target_electronics != -1);
    bool hardware_and_electronics_correct = metadata_handler_.CheckHardwareAndElectronics(target_hardware, target_electronics);
    //If the value we are meant to flash does not match the current motor throw a warning and don't allow flashing
    //A wrong value could be a mismatched Kv or incorrect motor type
    if(!(hardware_and_electronics_correct)){

        //Hardware name needs to either be from what's in the dropdown, or from the previously handled connection value
        QString hardwareName = iv.pcon->GetHardwareNameFromResources(target_hardware);

        //Determine which thing they have wrong
        QString errorType;
        errorType = metadata_handler_.GetErrorType(target_hardware, target_electronics);

        QMessageBox msgBox;
        msgBox.setWindowTitle("WARNING!");

        QString error_msg;

        if(should_put_hardware_electronics_in_msg){
            error_msg = "The firmware you are trying to flash is not meant for this motor. Please go to vertiq.co "
                         "and download the correct file for your motor: " + hardwareName + "\n\n" + "Error(s): " + errorType;
        }else{
            error_msg = "The recovery firmware selected was not meant for your module. Please go to vertiq.co and download the correct file for your module.";
        }

        msgBox.setText(error_msg);

        iv.pcon->AddToLog(error_msg.toLower());

        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        if(msgBox.exec() == QMessageBox::Ok){
            firmware_binary_button_->setText("Select Firmware (\".bin\") or (\".zip\")" );
            firmware_bin_path_ = "";
            ResetMetadata();
            return true;
        }
    }

    return false;
}

void Firmware::FlashCombinedClicked(){
    if(using_metadata_){
        QStringList flashTypes = metadata_handler_.GetFlashTypes();
        if(flashTypes.contains("main")){
            type_flash_requested_ = "main";
        }else{
            type_flash_requested_ = "combined";
        }
    }else{
        type_flash_requested_ = "combined";
    }

    iv.pcon->AddToLog("combined flash requested");
    FlashClicked();
}

void Firmware::FlashBootClicked() {
    type_flash_requested_= "boot";
    iv.pcon->AddToLog("boot flash requested");
    FlashClicked();
}

void Firmware::FlashAppClicked(){
    type_flash_requested_ = "app";
    iv.pcon->AddToLog("app flash requested");
    FlashClicked();
}

void Firmware::FlashUpgradeClicked() {
    type_flash_requested_= "upgrade";
    iv.pcon->AddToLog("upgrade flash requested");
    FlashClicked();
}

void Firmware::FlashClicked() {

    iv.pcon->AddToLog("flash initiated");

    //After you click Flash Combined, set the binary path to the combined binary
    //Check that you have a good file and are connected to a motor
    //Check that the hardware and electronics are correct
    //Flash the motor
    uint32_t startingMemoryLocation = DEFAULT_STARTING_LOCATION_;

    //Only do this check if actaully have metadata to look at and we're not in recovery
    //(Recovery doesn't know anything about the hardware or electronics)
    if(using_metadata_){
        //Save the new boot version to the motor
       firmware_bin_path_ = metadata_handler_.GetPathToCorrectBin(type_flash_requested_);

       //Extra checking. If we only have app and boot, but our json tells us we have all 3, make the app's starting
       //Location equal to the combined instead

       //If we want an app flash, there is no upgrader on the motor, and the app is in the 3rd index of the flash types of the json
       if((type_flash_requested_ == "app") && !upgrade_present_ && (app_index_ == 3)){
            startingMemoryLocation = metadata_handler_.GetStartingMemoryFromType("upgrade");
       }else{
            startingMemoryLocation = metadata_handler_.GetStartingMemoryFromType(type_flash_requested_);
       }

    }

    iv.pcon->AddToLog("flash starting at location: 0x" + QString("%1").arg(startingMemoryLocation, 0, 16));

    if(CheckPathAndConnection()){
        return;
    }

    FlashFirmware(startingMemoryLocation);
}

void Firmware::FlashFirmware(uint32_t startingPoint){

    int currentTab = iv.pcon->GetCurrentTab();
    QString selected_port_name;

    //Grab the correct port name
    if(currentTab == FIRMWARE_TAB){
        selected_port_name = iv.pcon->GetSelectedPortName();
    }else if(currentTab == RECOVERY_TAB){
        selected_port_name = iv.pcon->GetRecoveryPortName();
    }

    //only try to go into boot mode if we are trying to flash new firmware from a motor not in recovery
    if(currentTab == FIRMWARE_TAB){
        if (!BootMode()) {
            return;
        }
    }

    try {
      Schmi::ErrorHandlerStd error;
      Schmi::BinaryFileStd bin(firmware_bin_path_.toStdString());
      Schmi::QSerial ser(selected_port_name);
      FlashLoadingBar flashBar(flash_progress_bar_);
      FlashLoadingBar recoverBar(recover_progress_bar_);

      Schmi::FlashLoader * fl;

      if(currentTab == FIRMWARE_TAB){
          fl = new Schmi::FlashLoader(&ser, &bin, &error, &flashBar);
      }else if(currentTab == RECOVERY_TAB){
          fl = new Schmi::FlashLoader(&ser, &bin, &error, &recoverBar);
      }

      fl->Init();

      if(currentTab == FIRMWARE_TAB){
          std::chrono::steady_clock::time_point time_start = std::chrono::steady_clock::now();
          bool boot_mode = false;
          while (!boot_mode) {
            boot_mode = fl->InitUsart();
            if (std::chrono::steady_clock::now() - time_start > std::chrono::milliseconds(10000)) {
              QString error_msg("Could Not Init UART From Boot Mode");
              iv.pcon->AddToLog(error_msg.toLower());
              throw QString(error_msg);
              break;
            };
          }
      }

      bool init_usart = false;
      bool global_erase = false;
      fl->Flash(init_usart, global_erase, startingPoint);

      //We don't want people to hang out in the recovery page. so don't let them
      if(currentTab == RECOVERY_TAB){
          iv.pcon->EnableAllButtons();
          iv.pcon->ResetToTopPage();
      }

      //We are now done with the extracted directory that we made. We should delete it to avoid any issues
      ResetMetadata();

      //We're done with the flash loader now
      delete fl;

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

      QString status_msg("Waiting for motor to go in BootMode");
      iv.label_message->setText(status_msg);
      iv.pcon->AddToLog(status_msg.toLower());

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
    iv.pcon->AddToLog("could not go into boot mode: " + error_message);
    return 0;
  }
  return 1;
}

void Firmware::ResetMetadataOnConnection(){
    ResetMetadata();
}

//Clear out our locally stored metadata files and binaries, and mark that we are not currently
//using any metadata.
void Firmware::ResetMetadata(){
    if(using_metadata_){
      metadata_handler_.Reset(iv.pcon->GetMainWindowAccess());
    }

    using_metadata_ = false;
}

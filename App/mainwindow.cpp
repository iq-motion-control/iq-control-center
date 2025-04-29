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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common_icon_creation.h"
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {

  ui->setupUi(this);
  ui->stackedWidget->setCurrentIndex(0);
  ui->pushButton_home->setChecked(1);

  // Check the server for updates
  AutoCheckUpdate();

  // Connect the Help Buttons to the maintenance tool
  connect(ui->actionCheck_for_Updates, SIGNAL(triggered()), this, SLOT(updater()));
  connect(ui->actionImport_Resource_Pack, SIGNAL(triggered()), this, SLOT(importResourcePack()));

  //Place the GUI Version in the bottom left under the Information section
  QString gui_version =
      QString::number(MAJOR) + "." + QString::number(MINOR) + "." + QString::number(PATCH);
  ui->label_gui_version_value->setText(gui_version);

  try {
    // Create a ResourceFileHandler object and pass in the path to the SessionResourceFiles directory in AppData
    resource_file_handler = new ResourceFileHandler(appDataSessionResourcesPath);

    iv.pcon = new PortConnection(ui, resource_file_handler);
    iv.label_message = ui->header_error_label;


    //create our LocalData folder (definition comes from port connection (where logging happens)
    //Also create a folder to hold all of the user default files
    QDir appData, defaults_files;
    appData.mkdir(iv.pcon->app_data_folder_); // these will only create the folder once.
    appData.mkdir(iv.pcon->path_to_user_defaults_repo_);

    //Write that the control center opened to the log
    iv.pcon->logging_active_ = true;
    iv.pcon->AddToLog("IQ Control Center Opened with version " + gui_version);

    // Load default resource files that are packaged with the application
    loadDefaultResourceFiles();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), iv.pcon, SLOT(TimerTimeout()));
    timer->start(50);

    //Find available COM ports and display options in the PORT tab
    connect(ui->serial_port_combo_box, SIGNAL(CustomComboBoxSelected()), iv.pcon, SLOT(FindPorts()));
    connect(ui->serial_port_combo_box, QOverload<int>::of(&QComboBox::activated), iv.pcon,
            &PortConnection::PortComboBoxIndexChanged);

    //Connect "connect button" with the port connection module so that we can connect to the motor on button press
    connect(ui->connect_button, SIGNAL(clicked()), iv.pcon, SLOT(ConnectToSerialPort()));
    iv.pcon->FindPorts();
    //Baud Rate dropdown connect with actual baud rate for communication
    connect(ui->serial_baud_rate_combo_box, QOverload<int>::of(&QComboBox::activated), iv.pcon,
            &PortConnection::BaudrateComboBoxIndexChanged);
    iv.pcon->FindBaudrates();

    //Module ID Dropdown connect with actual module ID for communication
    connect(ui->selected_module_combo_box, QOverload<int>::of(&QComboBox::activated), iv.pcon,
            &PortConnection::ModuleIdComboBoxIndexChanged);
    //Connect the DETECT button with calling the detect modules on the bus function
    connect(ui->detect_button, SIGNAL(clicked()), iv.pcon, SLOT(DetectModulesClickedCallback()));

    //Because we set motors to an initial baud rate of 115200, we should display that as the default value in order
    //to reduce the number of clicks the user has to make in order to connect with the motor
    int index115200 = ui->serial_baud_rate_combo_box->findText("115200");
    ui->serial_baud_rate_combo_box->setCurrentIndex(index115200); //Set first shown value to 115200
    iv.pcon->BaudrateComboBoxIndexChanged(index115200); //Actually select the value as 115200

    //Connect a lost connection with the motor to clearing all tabs in the window
    connect(iv.pcon, SIGNAL(LostConnection()), this, SLOT(ClearTabs()));

    tab_populator = new TabPopulator(ui, resource_file_handler, &tab_map_);

    //Connects values between the tab populator and port connection. In this case, we are connecting firmware style, hardware type, firmware build, and firmware versioning style
    connect(iv.pcon, SIGNAL(TypeStyleFound(int,int,int,int,int,int)), tab_populator,
            SLOT(PopulateTabs(int,int,int,int,int,int)));

    connect(iv.pcon, SIGNAL(FindSavedValues()), this, SLOT(ShowMotorSavedValues()));

    def = new Defaults(ui->default_box, "/Resources/Defaults/", iv.pcon->path_to_user_defaults_repo_.toStdString());

    connect(ui->default_box, QOverload<int>::of(&QComboBox::activated), def,
            &Defaults::DefaultComboBoxIndexChanged);
    connect(ui->default_pushbutton, SIGNAL(clicked()), def, SLOT(LoadDefaultValues()));
    connect(def, SIGNAL(SaveDefaultValues(Json::Value)), this, SLOT(SetDefaults(Json::Value)));

    //initialize our firmware handler with the necessary information
    firmware_handler_.Init(ui->flash_progress_bar, ui->select_firmware_binary_button, ui->recovery_progress, ui->select_recovery_bin_button);

    connect(ui->flash_button, SIGNAL(clicked()), &firmware_handler_, SLOT(FlashCombinedClicked()));
    connect(ui->flash_boot_button, SIGNAL(clicked()), &firmware_handler_, SLOT(FlashBootClicked()));
    connect(ui->flash_app_button, SIGNAL(clicked()), &firmware_handler_, SLOT(FlashAppClicked()));
    connect(ui->flash_upgrade_button, SIGNAL(clicked()), &firmware_handler_, SLOT(FlashUpgradeClicked()));


    connect(ui->select_firmware_binary_button, SIGNAL(clicked()), &firmware_handler_,
            SLOT(SelectFirmwareClicked()));

    //Connect pressing the Recover file button with the firmware SelectRecoveryClicked to allow picking a file
    connect(ui->select_recovery_bin_button, SIGNAL(clicked()), &firmware_handler_, SLOT(SelectFirmwareClicked()));

    //Connect pressing the Recover button with the RecoverClicked() function to rescue the motor
    //Only allow recovery to flash a combined to avoid any more issue
    connect(ui->recover_button, SIGNAL(clicked()), &firmware_handler_, SLOT(FlashCombinedClicked()));

    //whenever I connect with a module, I want to reset our metadata
    //This protects against having bad firmware sticking around for people to flash to the wrong
    //type of module (specifically in recovery mode)
    //There's a couple of choices to fix the behavior:
    //1. erase/disable the connect button when we're in recovery. I don't love this one, because it completely traps
    //people in the recovery tab until they recover their module or restart the control center
    //2. do this. whenever someone connects a new module (or clicks the connect button while in the recovery tab)
    //just clear out whatever metadata/files may or may not be hanging around
    connect(ui->connect_button, SIGNAL(clicked()), &firmware_handler_, SLOT(ResetMetadata()));

    connect(ui->identify_button, SIGNAL(clicked()), iv.pcon, SLOT(PlayIndication()));

    //Set up shared icons
    icon_setup();

  } catch (const QString &e) {
    ui->header_error_label->setText(e);
    return;
  }
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::AutoCheckUpdate(){
    process = new QProcess(parent());
    QDir exe = QDir(QCoreApplication::applicationDirPath());
    QString file = exe.absoluteFilePath(MAINTENANCETOOL_PATH);
    QStringList arguments;
    arguments << MAINTENANCETOOL_FLAGS;
    process->start(file, arguments);
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(readOutput()));
}

void MainWindow::updater() {
    process = new QProcess(parent());
    QDir exe = QDir(QCoreApplication::applicationDirPath());
    QString file = exe.absoluteFilePath(MAINTENANCETOOL_PATH);
    QStringList arguments;
    arguments << "--su";
    process->start(file, arguments);
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
}

void MainWindow::importResourcePack() {
    ResourcePack * resourcePack = new ResourcePack(appDataSessionResourcesPath, appDataImportedResourcesPath);
//    resourcePack->displayMessageBox("Administrator privleges required", "If you did not run IQ Control Center as an administrator, please close this application and run it as an administrator. This is required to import a Resource Pack.");
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);

    QString openDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    //Open up the file window to let users pick the resource pack .zip file to import into Control Center
    QString zipFileToImport = dialog.getOpenFileName(this, ("Select Resource Pack .zip file"), openDir,
                                                          tr("Zip (*.zip)"));
    if(zipFileToImport != NULL){
      resourcePack->importResourcePackFromPath(zipFileToImport);
    }else{
      iv.pcon->AddToLog("Import Resource Pack clicked but no .zip file selected.");
    }
    delete resourcePack;
}

void MainWindow::loadDefaultResourceFiles(){
    iv.pcon->AddToLog("Loading resource files into: " + appDataSessionResourcesPath);
    // Create a QDir object with the AppData/SessionResourceFiles path defined in the constructor
    QDir appDataResourcesDirectory(appDataSessionResourcesPath);

    // Create a QDir object that represents the main Resources directory of the Control Center app
    // This object iterates through each directory and file, including subdirectories
    QDirIterator dirIterator(mainResourcesDirectory,
                             QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                             QDirIterator::Subdirectories);

    while(dirIterator.hasNext()){
      // Grab the next file/directory in the Resources directory
      dirIterator.next();
      // Create a fileInfo object to get the name of the file/directory
      QFileInfo fileInfo = dirIterator.fileInfo();
      // Only look at non-hidden files/directories
      if(!fileInfo.isHidden()){
        QString sourcePath = fileInfo.absoluteFilePath();
        // Get the name of the file/directory minus everything before the "Resources" directory
        QString fileName = sourcePath.mid(mainResourcesDirectory.length());
        // Construct the destination path where the file/directory is created in AppData
        QString destinationPath = appDataSessionResourcesPath + fileName;

        // Create the directory in AppData if the file is a directory
        if (fileInfo.isDir()){
          appDataResourcesDirectory.mkpath(destinationPath);
        } else {
        // Copy the file to AppData if it is a file
          // Need to remove any existing file or else copy will fail
          QFile::remove(destinationPath);
          QFile::copy(sourcePath, destinationPath);
        }
      }
    }
}

void MainWindow::readOutput() {
    std::string data;
    std::string error;

    data.append(process->readAllStandardOutput());
    error.append(process->readAllStandardError());

    if((data.find("no updates available") != std::string::npos) || (error.find("no updates available") != std::string::npos)){
        ui->header_error_label->setText("No Updates Available");
    }
    else if ((data.find("<updates>") != std::string::npos) || (error.find("<updates>") != std::string::npos)){
        ui->header_error_label->setText("UPDATE AVAILABLE: CLICK MENU IN TOP LEFT");
        show_update_message_box();
    }
    else{
        ui->header_error_label->setText("Error Checking For Updates!");
    }
    delete process;
}

void MainWindow::show_update_message_box(){
    QMessageBox msgBox;
    msgBox.setWindowTitle("Software Update");
    msgBox.setText(
        "A new IQ Control Center update is now available.\n"
        "Please navigate to 'Check for Updates' under Menu in the top left corner of the application.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::on_pushButton_home_clicked(){
    if(ui->stackedWidget->currentIndex() != RECOVERY_TAB){
        ui->stackedWidget->setCurrentIndex(0);
    }
}


void MainWindow::on_pushButton_general_clicked(){
    if(ui->stackedWidget->currentIndex() != RECOVERY_TAB){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MainWindow::on_pushButton_tuning_clicked(){
    if(ui->stackedWidget->currentIndex() != RECOVERY_TAB){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void MainWindow::on_pushButton_testing_clicked(){
    if(ui->stackedWidget->currentIndex() != RECOVERY_TAB){
        ui->stackedWidget->setCurrentIndex(3);
    }
}

void MainWindow::on_pushButton_get_help_clicked(){
    if(ui->stackedWidget->currentIndex() != RECOVERY_TAB){
        ui->stackedWidget->setCurrentIndex(7);
    }
}

void MainWindow::on_pushButton_firmware_clicked() {
    if(ui->stackedWidget->currentIndex() != RECOVERY_TAB){
        ui->flash_progress_bar->reset();

        //Make all of the flash buttons invisible
        ui->flash_app_button->setVisible(false);
        ui->flash_boot_button->setVisible(false);
        ui->flash_button->setVisible(false);
        ui->flash_upgrade_button->setVisible(false);

        ui->stackedWidget->setCurrentIndex(4);
    }
}

void MainWindow::on_pushButton_advanced_clicked() {
    if(ui->stackedWidget->currentIndex() != RECOVERY_TAB){
        QMessageBox msgBox;
        msgBox.setWindowTitle("WARNING!");
        msgBox.setText(
         "Changing settings in the Advanced tab could compromise the safety features that Vertiq has put in "
         "place. Please use extreme caution.\n\nI understand that changing settings in this tab could "
         "result in the damage or destruction of my motor.");
        msgBox.setStandardButtons(QMessageBox::Yes);
        msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        if (msgBox.exec() == QMessageBox::Yes) {
            ui->stackedWidget->setCurrentIndex(5);
        }else{
            ui->stackedWidget->setCurrentIndex(0);
            ui->pushButton_advanced->setChecked(false);
            ui->pushButton_home->setChecked(true);
        }
    }
}

void MainWindow::ShowMotorSavedValues() {
  if (iv.pcon->GetConnectionState() == 1) {
    for (std::pair<std::string, std::shared_ptr<Tab>> tab : tab_map_) {
      tab.second->CheckSavedValues();
    }
  } else {
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.label_message->setText(error_message);
  }
}

void MainWindow::HandleDefaultsPopup(bool unable_to_reboot){
    QString success_message = "Default Settings Values Saved";
    iv.label_message->setText(success_message);
    iv.pcon->AddToLog(success_message);

    //Give the user the option to reboot the module after setting with defaults.
    QMessageBox msgBox;
    QAbstractButton * rebootButton = msgBox.addButton("Reboot Now", QMessageBox::YesRole);
    QAbstractButton * exitButton = msgBox.addButton("Do Not Reboot Now", QMessageBox::NoRole);

    msgBox.setWindowTitle("Defaults Set Successfully");

    QString text;

    if(!unable_to_reboot){
        text.append("Set values from default file successfully. We recommend that you restart your module"
                    " to ensure that all changes take effect. If you would like to restart your module now,"
                    " please select Reboot Now. Your module will disconnect from IQ Control Center.");
    }else{
        text.append("Set values from default file successfully. Due to the values changed, your "
                    "module has disconnected, and we are unable to reboot your module through IQ Control Center. We highly "
                    "recommend that you manually power cycle your module to ensure that all new parameters take effect.");

        msgBox.removeButton(rebootButton);
        exitButton->setText("OK");
    }

    msgBox.setText(text);
    msgBox.exec();

    if(msgBox.clickedButton() == rebootButton){
        //reboot the motor to make sure all changes take full effect (specifically is module id gets changed)
        iv.pcon->RebootMotor();
    }else{
        iv.pcon->AddToLog("module not rebooted after setting through defaults");
    }
}

bool MainWindow::ReadAndPopulateDefaults(Json::Value defaults){
    iv.pcon->AddToLog("setting through defaults");

    //create a map of the values in the defaults file (descriptor and value)
    std::map<std::string, double> default_value_map;

    //Go through each of the entries in the input json
    for (uint8_t ii = 0; ii < defaults.size(); ++ii) {

      //If the current entry has an Entries object keep going
      if (!defaults[ii]["Entries"].empty()) {

          //Clear out the maps to have a fresh slate for this tab's frames
          default_value_map.clear();

          //the tab we should be looking for is the descriptor of this object (general, tuning, etc.)
          //the value will actually be something like advanced_vertiq_40xx_speed.json
          std::string tab_descriptor = defaults[ii]["descriptor"].asString() + ".json";

          //Save our tab descriptors for later (don't want to bother finding them again with the special defaults)
          tab_descriptors.append(tab_descriptor.c_str());
          if(tab_descriptor.find("advanced") != std::string::npos){
            advanced_index = ii;
          }else if(tab_descriptor.find("tuning") != std::string::npos){
            tuning_index = ii;
          }else if(tab_descriptor.find("general") != std::string::npos){
            general_index = ii;
          }

          //The tab's default values are stored in the Entries array
          Json::Value defaults_values = defaults[ii]["Entries"];

          //For each of the values in the Entries array
          for (uint8_t jj = 0; jj < defaults_values.size(); ++jj) {
             //Grab the desciptor for each entry and its value
            std::string value_descriptor = defaults_values[jj]["descriptor"].asString();
            double value = defaults_values[jj]["value"].asDouble();

            //Put the value into the correct map with its descriptor as the key
            if(!SPECIAL_DEFAULTS.contains(QString(value_descriptor.c_str()))){
              default_value_map[value_descriptor] = value;
            }else if(tab_descriptor.find("advanced") != std::string::npos){
              advanced_special_value_map[value_descriptor] = value;
            }else if(tab_descriptor.find("tuning") != std::string::npos){
                tuning_special_value_map[value_descriptor] = value;
            }else if(tab_descriptor.find("general") != std::string::npos){
                general_special_value_map[value_descriptor] = value;
            }

          }

          //Create a map iterator, and look to see if the tab targeted by this defaults file matches
          //what's in our tab_map_
          std::map<std::string, std::shared_ptr<Tab>>::const_iterator it;
          it = tab_map_.find(tab_descriptor);

          //If you find the tab you're looking for yay. If not, then throw an error and don't do anything else
          if (it != tab_map_.end()) {
            //yay, you found the right tab. Now set the value in the tab to what was given
            //in the defaults file, and make sure the correct values appear in the gui

            iv.pcon->AddToLog("setting " + QString(tab_descriptor.c_str()) + " values through defaults\n");

            tab_map_[tab_descriptor]->SaveDefaults(default_value_map);

            iv.pcon->AddToLog("checking " + QString(tab_descriptor.c_str()) + " values after setting through defaults\n");
            tab_map_[tab_descriptor]->CheckSavedValues();

          } else {

            QString error_message = "Wrong Default Settings Selected";
            iv.label_message->setText(error_message);
            iv.pcon->AddToLog(error_message);
            return false;
          }
        }
    }

    return true;
}

void MainWindow::SetDefaults(Json::Value defaults) {

    //Keep track of if we changed the baud rate, or any other variables that'll stop us from rebooting
    bool unable_to_reboot = false;

  //if the motor is connected, and you have a non-empty tab_map_
  if (iv.pcon->GetConnectionState() == 1 && !tab_map_.empty()) {

    //Handle our standard defaults (the ones that won't break anything)
    if(!ReadAndPopulateDefaults(defaults)){
        return;
    }

    //Now that we've handled the normal vars, let's handle the special ones
    unable_to_reboot = this->HandleSpecialDefaults();

    //Handle the display after we've set all of the new values through defaults
    HandleDefaultsPopup(unable_to_reboot);

    return;

  } else {
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.label_message->setText(error_message);
    return;
  }
}

bool MainWindow::HandleSpecialDefaults() {

    iv.pcon->AddToLog("setting special defaults\n");

    //The Json::Value defaults holds all of the information held in the Defaults file json input
    //We've already got the special frames that we care about stored.
    //We also have the names of each of the tab_descriptors stored
    //We also know which index each of the tab descriptors lives in
    bool tuning_cant_restart = tab_map_[tab_descriptors.at(tuning_index).toStdString()]->SaveSpecialDefaults(tuning_special_value_map);
    bool general_cant_restart = tab_map_[tab_descriptors.at(general_index).toStdString()]->SaveSpecialDefaults(general_special_value_map);
    bool advanced_cant_restart = tab_map_[tab_descriptors.at(advanced_index).toStdString()]->SaveSpecialDefaults(advanced_special_value_map);

    return tuning_cant_restart || general_cant_restart || advanced_cant_restart;
}


void MainWindow::ClearTabs() { tab_map_.clear(); }

void MainWindow::write_version_info_to_file(QJsonArray * json_array){
    //Append the version information (firmware, hardware, control center)
    QJsonObject version_information, build_info;

    version_information.insert("Firmware Build", ui->label_firmware_build_value->text());
    version_information.insert("GUI Version", ui->label_gui_version_value->text());
    version_information.insert("Firmware Name", ui->label_firmware_name->text());
    version_information.insert("Hardware Name", ui->label_hardware_name->text());
    version_information.insert("Bootloader Version", ui->label_bootloader_value->text());
    version_information.insert("Upgrader Version", ui->label_upgrader_value->text());

    // Get the hardware, electronics, and firmware types and major versions from the PortConnection object
    int hardwareType = iv.pcon->GetHardwareType();
    int hardwareMajorVersion = iv.pcon->GetHardwareMajorVersion();
    int electronicsType = iv.pcon->GetElectronicsType();
    int electronicsMajorVersion = iv.pcon->GetElectronicsMajorVersion();
    int firmwareType = iv.pcon->GetFirmwareStyle();

    // Generate SKU based on hardware, electronics, and firmware types and major versions
    // ex: M16.2-E16.2-F1
    QString sku = "M" + QString::number(hardwareType) + "." + QString::number(hardwareMajorVersion) + "-E" + QString::number(electronicsType) + "." + QString::number(electronicsMajorVersion) + "-F" + QString::number(firmwareType);
    // Insert SKU into Build information section of support file
    version_information.insert("SKU:", sku);

    build_info.insert("Build information", version_information);

    json_array->append(build_info);
}

void MainWindow::write_parameters_to_file(QJsonArray * json_array, exportFileTypes exportStyle){
    //Now go through and get all of the values that are currently set on the control center/module

    //If we're connected, then we have a map of tabs (general, tuning, advanced, testing)
    //Tabs store all of our Frames (velocity kd, timeout, Voltage, etc.)
    //Each Frame stores the value that we got from the motor
    std::map<QString, std::map<int, QString>> frameVariablesMap;

    for (std::pair<std::string, std::shared_ptr<Tab>> tab : tab_map_) {
        // Get the frame_variables_map_ from each tab object.
        // This contains the client, and the list_names and list_values that will be used to get the readable client entry value
        std::map<std::string,FrameVariables*> fv = tab.second->get_frame_variables_map();
        for(const auto& elem : fv){
            // Only the comboboxes have list_names
            if (elem.second->frame_type_ == 1){
                // Get the name of the client
                QString client = QString::fromStdString(elem.first);
                // Create the inner map object that holds the list_value and list_name
                std::map<int, QString> valueNameMap;
                for (uint8_t listNameIndex = 0; listNameIndex < elem.second->combo_frame_.list_names.size(); listNameIndex++) {
                    // This is the list_name, which is a client endpoint value
                    QString value_string = QString::fromStdString(elem.second->combo_frame_.list_names[listNameIndex]);
                    // This is the enumerated number that represents the client endpoint value
                    int value_number = elem.second->combo_frame_.list_values[listNameIndex];
                    // Insert this pairing into the map object that holds the mapping between list_value and list_name for this client
                    valueNameMap.insert({value_number, value_string});
                }
                // Insert the client and its value map into the final outer object
                frameVariablesMap.insert({client, valueNameMap});
            }
        }
      //If we're doing a defaults file export, we don't want anything to do with the testing tab.
      //This could be especially dangerous if the file gets saved with say 1000rpm and someone loads it
      //with a prop on
      if(!((exportStyle == exportFileTypes::DEFAULTS_FILE) && (tab.first.find("testing") != std::string::npos))){
          QJsonObject top_level_tab_obj;

          QJsonArray tab_frame_array;

          //Grab the frame map from the current tab
          std::map<std::string, Frame *> frames_in_tab = tab.second->get_frame_map();

          //Go through each frame in the tab
          for(auto frame = frames_in_tab.begin(); frame != frames_in_tab.end(); frame++){
            //Grab the frame as the top level, and cast it later
            Frame * curFrame = frame->second;

            //Create a new object to hold the tab frame object. DON'T FORGET TO DELETE AT THE END OF THE LOOP
            QJsonObject * current_tab_json_object = new QJsonObject();

            //Only add the frames that we care about.
            bool attach_new_object = false;

            switch(curFrame->frame_type_){
              case 1:
              {
                  FrameCombo *fc = (FrameCombo *)(curFrame);
                  // Use the map object we created earlier to get the list_name for the list_value of this client
                  QString readable_value = frameVariablesMap[frame->first.c_str()][fc->value_];
                  current_tab_json_object->insert("value", fc->value_);
                  current_tab_json_object->insert("readable_value", readable_value);
                  attach_new_object = true;
                break;
              }
              case 2:
              {
                  FrameSpinBox *fsb = (FrameSpinBox *)(curFrame);
                  current_tab_json_object->insert("value", fsb->value_);
                  attach_new_object = true;
                break;
              }
              case 3:
              {
                //Switch frame, don't do anything
                break;
              }
              case 4:
              {
                  FrameTesting *ft = (FrameTesting *)(curFrame);
                  current_tab_json_object->insert("value", ft->value_);
                  attach_new_object = true;
                break;
              }
              case 5:
              {
                  //Button frame, do nothing
                break;
              }

              case 6:
              {
                FrameReadOnly *fr = (FrameReadOnly *)(curFrame);
                current_tab_json_object->insert("value", fr->value_);
                attach_new_object = true;
                break;
              }
            }

            if(attach_new_object){

                //NOTE:: It does not matter where baud rate goes in the advanced tab json entry. When we set through
                //       defaults, we read the json as a map sorted alphabetically. This will always put baud rate
                //       near the top.
                current_tab_json_object->insert("descriptor", frame->first.c_str());
                tab_frame_array.append(*current_tab_json_object);
            }

            delete current_tab_json_object;
          }

          uint8_t extension_index = tab.first.find(".");
          std::string tab_descriptor = tab.first.substr(0, extension_index);

          //Fill in with "entries" and "descriptors" so that the file is filled in the order matching our defaults files
          //If we fill "Entries" directly, it will be inserted above descriptor. While it wouldn't make a functional difference
          //It would be harder to read for a human
          top_level_tab_obj.insert("entries", tab_frame_array);
          top_level_tab_obj.insert("descriptor", tab_descriptor.c_str());

          //Write to our output array
          json_array->append(top_level_tab_obj);
      }

    }
}

void MainWindow::write_metadata_to_file(QJsonArray * json_array){
    QJsonObject output_metadata_object;
    QJsonObject metadata;

    uint32_t uid1, uid2, uid3;

    metadata.insert("Generated Date and Time", iv.pcon->time_.currentDateTime().toString(Qt::TextDate));

    iv.pcon->GetUidValues(&uid1, &uid2, &uid3);
    metadata.insert("UID1", (int)uid1);
    metadata.insert("UID2", (int)uid2);
    metadata.insert("UID3", (int)uid3);

    output_metadata_object.insert("Metadata", metadata);

    json_array->append(output_metadata_object);
}

void MainWindow::write_user_support_file(){
    QJsonArray tab_array;

    write_metadata_to_file(&tab_array);
    write_version_info_to_file(&tab_array);
    write_parameters_to_file(&tab_array, exportFileTypes::SUPPORT_FILE);

    write_data_to_json(tab_array, exportFileTypes::SUPPORT_FILE);
}

bool MainWindow::compressSupportFiles(QString supportFilePath, QStringList supportFiles){
    JlCompress compressTool;
    QString compressPath = supportFilePath;
    // The supportFilePath will either contain .txt or .json since this file needs to be generated before being zipped
    // The .zip file that will be generated will use the same name as the support file.
    if(compressPath.contains(".txt")){
      compressPath = compressPath.replace(".txt", ".zip");
    }
    if(compressPath.contains(".json")){
      compressPath = compressPath.replace(".json", ".zip");
    }
    return compressTool.compressFiles(compressPath, supportFiles);
}

void MainWindow::on_generate_support_button_clicked(){
    //If we're connected then go get the values, otherwise just spit out a message to connect the motor
    if (iv.pcon->GetConnectionState() == 1) {
        write_user_support_file();
    }else{
        // Allow user to specify where the log file should be saved
        QMessageBox disconnectedWarningMessageBox;
        disconnectedWarningMessageBox.setIcon(QMessageBox::Warning);
        disconnectedWarningMessageBox.setWindowTitle("No Module Connected!");
        QString disconnectedWarning = "There is currently no module connected. Logs from Control Center can still be generated as part of a support package, "
            "but the support information from a specific motor will not be generated. If possible, we recommend connecting a motor before generating a support file "
            "to generate the most complete support information. Press 'OK' to continue generating the log file without connecting your module, or 'Cancel' and try again "
            "after connecting your module.";
        disconnectedWarningMessageBox.setText(disconnectedWarning);
        disconnectedWarningMessageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
        if(disconnectedWarningMessageBox.exec() == QMessageBox::Ok){
            QString tempLogFilePath = QFileDialog::getSaveFileName(this, tr("Open Directory"), "/home/user_support_log", tr("zip (*.zip"));

            // Although the file dialog displayed .zip as the filetype, the log file must be saved as a .txt file before being compressed into a .zip file
            if(tempLogFilePath.length() > 0){
              tempLogFilePath = tempLogFilePath.replace(".zip", ".txt");
              QString logPath = exportLog(tempLogFilePath);

              QMessageBox msgBox;
              msgBox.setWindowTitle("Generate Log File");

              QString text;

              QStringList support_files = {logPath};
              QString compressPath = logPath;

              // Set required permissions for the path of the .zip file that will be created
              QFile file(compressPath);
              file.setPermissions(compressPath, QFileDevice::ReadOther | QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadUser
                                                    | QFileDevice::WriteOther | QFileDevice::WriteOwner | QFileDevice::WriteGroup | QFileDevice::WriteUser
                                                    | QFileDevice::ExeOther | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeUser
                                  );

              if (compressSupportFiles(compressPath, support_files)){
                  logPath = logPath.replace(".txt", ".zip"); // Replace the .txt extension with .zip since the compressed file extension is .zip
                  text.append("Your log file has been successfully generated at: " + logPath + ". "
                              "If you are not already in contact with a member of the Vertiq support team, please email the .zip file "
                              "to support@vertiq.co with your name and complication, and we will respond as soon as possible.");

                  iv.pcon->AddToLog(text);
                  msgBox.setStandardButtons(QMessageBox::Ok);

                // Remove the .txt file because the .zip file already contains the log file
                if(QFile::exists(tempLogFilePath)){
                    QFile::remove(tempLogFilePath);
                }
              }
              file.close();
              msgBox.setText(text);
              msgBox.exec();
            }
        }
    }
}

void MainWindow::display_successful_import(){
    QString success_message("Custom defaults imported properly");
    iv.label_message->setText(success_message);
    iv.pcon->AddToLog(success_message);
    def->RefreshFilesInDefaults();
}

void MainWindow::import_defaults_file_from_path(QString json_to_import){
    //if you actually picked a file
    if(json_to_import != ""){
        //Create a file from the path
        QFile defaults_file(json_to_import);

        //Also get the file info so we can extract just the name and not the whole path (ex. defauts.json)
        QFileInfo file_info(json_to_import);
        QString path_to_copy_to(iv.pcon->path_to_user_defaults_repo_ + "/" + file_info.fileName());

        iv.pcon->AddToLog("Copying defaults file to: " + path_to_copy_to);

        bool copySuccessful = QFile::copy(defaults_file.fileName(), path_to_copy_to);

        //Say that the file was imported properly, and then refresh the defaults dropdown
        if(copySuccessful){
            display_successful_import();
            iv.pcon->AddToLog("successfully added a custom defaults file at: " + json_to_import);
        }else{
            //Pop up a window that says it seems like a file with that name already exists...rename it please. also
            //give the option to overwrite the old one
            QMessageBox msgBox;
            msgBox.setWindowTitle("Defaults Import Error");

            iv.pcon->AddToLog("Failed to import: " + json_to_import + ". A defaults file with that name already exists.");

            QString text("A defaults file with the same name already exists. To"
                         " overwrite the existing file please select the \"Overwrite\" button. Otherwise, please "
                         "select \"Cancel,\" rename your file, and try importing it again.");

            msgBox.setText(text);
            msgBox.addButton("Cancel", QMessageBox::NoRole);
            QAbstractButton * overwriteButton = msgBox.addButton("Overwrite", QMessageBox::YesRole);

            //If you click cancel, don't do anything.
            //If you click overwrite, then delete the current file, and write this one
            msgBox.exec();
            if(msgBox.clickedButton() == overwriteButton){
                //delete the old version of this name of file
                QFile fileToDelete(path_to_copy_to);
                fileToDelete.setPermissions(path_to_copy_to, QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteGroup);
                fileToDelete.remove();

                //copy over the new one
                QFile::copy(defaults_file.fileName(), path_to_copy_to);

                iv.pcon->AddToLog("Overwrote: " + json_to_import + " with a new version of the file.");

                display_successful_import();
            }
        }
    }
}

void MainWindow::on_import_defaults_pushbutton_clicked(){

    //Note::Testing indicates that updating the control center will not overwrite/delete user files, but it is something we should check on
    //new releases from now on.

    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString openDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    //Open up the file window to let people pick the json they want to add to the Control Center
    QString json_to_import = QFileDialog::getOpenFileName(0, ("Select Defaults JSON File"), openDir,
                                                          tr("JSON (*.json)"));

    import_defaults_file_from_path(json_to_import);
}

void MainWindow::on_export_defaults_pushbutton_clicked(){
    //This is a lot like exporting the support file, only we don't need the metadata or build info
    QJsonArray tab_array;

    //If we're connected then go get the values, otherwise just spit out a message to connect the motor
    if (iv.pcon->GetConnectionState() == 1) {
        write_parameters_to_file(&tab_array, exportFileTypes::DEFAULTS_FILE);
        write_data_to_json(tab_array, exportFileTypes::DEFAULTS_FILE);
    }else{
        ui->header_error_label->setText("Please connect your module before attempting to generate a custom defaults file.");
    }

}

void MainWindow::write_data_to_json(QJsonArray tab_array, exportFileTypes fileExport){
    QString file_beginning;
    QString supportFilePath = "";

    switch(fileExport){
        case exportFileTypes::SUPPORT_FILE:{
            file_beginning.append("user_support_file_");
            // The FileDialog box will indicate that the generated Suppport file is a .zip file
            // The user can decide where to save this .zip file
            supportFilePath = QFileDialog::getSaveFileName(this, tr("Open Directory"),
                                                "/home/" + file_beginning + ui->label_firmware_name->text() + ".zip",
                                                tr("zip (*.zip"));
            // Although we are creating a .zip in the end, we need to save the support file as a .json to include it in the .zip file
            supportFilePath.replace(".zip", ".json");
            break;
        }

        case exportFileTypes::DEFAULTS_FILE:{
            file_beginning.append("custom_defaults_");
            supportFilePath = QFileDialog::getSaveFileName(this, tr("Open Directory"),
                                                "/home/" + file_beginning + ui->label_firmware_name->text() + ".json",
                                                tr("json (*.json"));
            break;
        }
    }

    //Write to the json file
    QJsonDocument output_doc;
    output_doc.setArray(tab_array);

    QByteArray bytes = output_doc.toJson(QJsonDocument::Indented);

    //Above we filled the array with "entries" not "Entries," but to match our defaults format, we want it to say "Entries"
    QString text(bytes); // add to text string for easy string replace
    text.replace("entries", "Entries");

    QFile file(supportFilePath);

    file.setPermissions(supportFilePath, QFileDevice::ReadOther | QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadUser
                                             | QFileDevice::WriteOther | QFileDevice::WriteOwner | QFileDevice::WriteGroup | QFileDevice::WriteUser
                                             | QFileDevice::ExeOther | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeUser
                        );

    if(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        QTextStream iStream( &file );
        iStream.setCodec( "utf-8" );
        iStream << text;

        QMessageBox msgBox;
        msgBox.setWindowTitle("File Generated");

        QString text;
        file.close();

        switch(fileExport){
            case exportFileTypes::SUPPORT_FILE: {
                QString logPath = exportLog(supportFilePath); // Also save log file with the at the same location

                QStringList supportFiles = {supportFilePath, logPath};
                QString compressPath = supportFilePath;
                QFile compressFile(compressPath);
                compressFile.setPermissions(compressPath, QFileDevice::ReadOther | QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadUser
                                                         | QFileDevice::WriteOther | QFileDevice::WriteOwner | QFileDevice::WriteGroup | QFileDevice::WriteUser
                                                         | QFileDevice::ExeOther | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeUser
                                    );
                compressFile.close();
                if (compressSupportFiles(compressPath, supportFiles)){
                    text.append("Your support file has been succesfully generated at: " + compressPath + ". "
                                "If you are not already in contact with a member of the Vertiq support team, please email the .zip file "
                                "to support@vertiq.co with your name and complication, and we will respond as soon as possible.");

                    msgBox.setStandardButtons(QMessageBox::Ok);
                }else{
                    text.append("Could not generate support file. Please contact the Vertiq support team at support@vertiq.co with your name and complication, "
                        "and we will respond as soon as possible.");

                }
                // Remove the support files that were used to .zip file compression to reduce duplicate files.
                if(QFile::exists(supportFilePath)){
                  QFile::remove(supportFilePath);
                }
                if(QFile::exists(logPath)){
                  QFile::remove(logPath);
                }
                break;
            }
            case exportFileTypes::DEFAULTS_FILE: {
                //If someone exports their defaults file, they'll probably want to use it again later on. So give them that option.
                text.append("Your module's current state has been saved in " + supportFilePath + ". Would you like to add these defaults"
                            " to the Control Center now? If no, you will have to use the Import button to do so manually later.");

                msgBox.setStandardButtons(QMessageBox::No);
                msgBox.addButton(QMessageBox::Yes);
                break;
            }
        }
        msgBox.setText(text);
        //If they click yes to import their export, then we'll have to directly import the file into the Defaults folder
        if(msgBox.exec() == QMessageBox::Yes){
            iv.pcon->AddToLog("Importing defaults file from current module state.");
            import_defaults_file_from_path(supportFilePath);
        }
    } else {
       QString error_text("Unable to open output file location, please try again.");
       ui->header_error_label->setText(error_text);
       iv.pcon->AddToLog(error_text);
    }
}

QString MainWindow::exportLog(QString path){
    //Grab the project log
    QFile currentLog(iv.pcon->path_to_log_file);
    QString logFilePath = path;

    if(logFilePath.contains(".json")){
        logFilePath = logFilePath.replace(".json", "_log.txt");
    }

    QFile logFile(logFilePath);
    logFile.setPermissions(logFilePath, QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteGroup |
                              QFileDevice::ReadOwner | QFileDevice::ReadUser | QFileDevice::ReadGroup);

    if(!(logFilePath.isEmpty())){
        // Remove the log file if it already exists or else copy will fail
        if(QFile::exists(logFilePath)){
            QFile::remove(logFilePath);
        }

        //Copy the data from the project log to the user's desired location
        if(currentLog.copy(logFilePath)){
            iv.pcon->AddToLog("Your log file has been succesfully exported.");
        }else{
            iv.pcon->AddToLog("Failed to export log.");
        }
    }else{
        ui->header_error_label->setText("Unable to open output file location, please try again.");
    }
  return logFilePath;
}

void MainWindow::on_clear_log_button_clicked(){
    ui->log_text_browser->clear();
}

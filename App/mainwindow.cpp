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

  //Place the GUI Version in the bottom left under the Information section
  QString gui_version =
      QString::number(MAJOR) + "." + QString::number(MINOR) + "." + QString::number(PATCH);
  ui->label_gui_version_value->setText(gui_version);

  try {
    iv.pcon = new PortConnection(ui);
    iv.label_message = ui->header_error_label;

    //create our LocalData folder (definition comes from port connection (where logging happens)
    //Also create a folder to hold all of the user default files
    QDir appData, defaults_files;
    appData.mkdir(iv.pcon->app_data_folder_); // these will only create the folder once.
    appData.mkdir(iv.pcon->path_to_user_defaults_repo_);

    //Write that the control center opened to the log
    iv.pcon->logging_active_ = true;
    iv.pcon->AddToLog("IQ Control Center Opened with version " + gui_version);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), iv.pcon, SLOT(TimerTimeout()));
    timer->start(1000);

    //Find available COM ports and display options in the PORT tab
    connect(ui->header_combo_box, SIGNAL(CustomComboBoxSelected()), iv.pcon, SLOT(FindPorts()));
    connect(ui->header_combo_box, QOverload<int>::of(&QComboBox::activated), iv.pcon,
            &PortConnection::PortComboBoxIndexChanged);

    //Connect "connect button" with the port connection module so that we can connect to the motor on button press
    connect(ui->header_connect_button, SIGNAL(clicked()), iv.pcon, SLOT(ConnectMotor()));
    iv.pcon->FindPorts();

    //Baud Rate dropdown connect with actual baud rate for communication
    connect(ui->header_baudrate_combo_box, QOverload<int>::of(&QComboBox::activated), iv.pcon,
            &PortConnection::BaudrateComboBoxIndexChanged);
    iv.pcon->FindBaudrates();
    //Because we set motors to an initial baud rate of 115200, we should display that as the default value in order
    //to reduce the number of clicks the user has to make in order to connect with the motor
    int index115200 = ui->header_baudrate_combo_box->findText("115200");
    ui->header_baudrate_combo_box->setCurrentIndex(index115200); //Set first shown value to 115200
    iv.pcon->BaudrateComboBoxIndexChanged(index115200); //Actually select the value as 115200

    //Connect a lost connection with the motor to clearing all tabs in the window
    connect(iv.pcon, SIGNAL(LostConnection()), this, SLOT(ClearTabs()));
    tab_populator = new TabPopulator(ui, &tab_map_);

    //Connects values between the tab populator and port connection. In this case, we are connecting firmware style, hardware type, firmware build, and firmware versioning style
    connect(iv.pcon, SIGNAL(TypeStyleFound(int,int,int)), tab_populator,
            SLOT(PopulateTabs(int,int,int)));

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
    }
    else{
        ui->header_error_label->setText("Error Checking For Updates!");
    }
    delete process;
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

void MainWindow::SetDefaults(Json::Value defaults) {

    //Keep track of if we changed the baud rate, and all of the information needed to write a new one
    bool update_baud = false;
    double baud_rate_in_defaults = 0;
    int baud_rate_frame_type = 0;
    Frame * baud_rate_frame;
    std::string baud_tab_descriptor;

  //if the motor is connected, and you have a non-empty tab_map_
  if (iv.pcon->GetConnectionState() == 1 && !tab_map_.empty()) {

    iv.pcon->AddToLog("setting through defaults");

    //create a map of the values in the defaults file (descriptor and value)
    std::map<std::string, double> default_value_map;

    //Go through each of the entries in the input json
    for (uint8_t ii = 0; ii < defaults.size(); ++ii) {

      //If the current entry has an Entries object keep going
      if (!defaults[ii]["Entries"].empty()) {

          //Clear out the default_value_map to have a fresh slate
        default_value_map.clear();

        //the tab we should be looking for is the descriptor of this object (general, tuning, etc.)
        std::string tab_descriptor = defaults[ii]["descriptor"].asString() + ".json";

        //The tab's default values are stored in the Entries array
        Json::Value defaults_values = defaults[ii]["Entries"];

        //For each of the values in the Entries array
        for (uint8_t jj = 0; jj < defaults_values.size(); ++jj) {
           //Grab the desciptor for each entry and its value
          std::string value_descriptor = defaults_values[jj]["descriptor"].asString();
          double value = defaults_values[jj]["value"].asDouble();

          //Put the value into the map with its descriptor as the key
          default_value_map[value_descriptor] = value;
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

          //We need some temp vars to track the baud rate information from this tab
          bool temp_baud_changed = false;
          int temp_baud_frame_type = 0;
          double temp_baud_to_set = 0;

          Frame * temp_ptr = tab_map_[tab_descriptor]->SaveDefaults(default_value_map, &temp_baud_changed, &temp_baud_frame_type, &temp_baud_to_set);

          //If we got the baud rate frame, then store it
          if(temp_ptr != nullptr){
            baud_rate_frame = temp_ptr;
          }

          //If the baud changed, then store all of the information for later
          if(temp_baud_changed){
              update_baud = true;
              baud_rate_frame_type = temp_baud_frame_type;
              baud_rate_in_defaults = temp_baud_to_set;
              baud_tab_descriptor = tab_descriptor;
          }

          iv.pcon->AddToLog("checking " + QString(tab_descriptor.c_str()) + " values after setting through defaults\n");
          tab_map_[tab_descriptor]->CheckSavedValues();

        } else {

          QString error_message = "Wrong Default Settings Selected";
          iv.label_message->setText(error_message);
          iv.pcon->AddToLog(error_message);
          return;
        }
      }
    }

    //We have now saved and checked all defaults besides the baud rate, but we've stored all important data about the baud rate
    //so we can safely change it now
    if(update_baud){
        iv.pcon->AddToLog("Setting a new baud rate: " + QString::number(baud_rate_in_defaults));
        //grab the tab where baud rate lives and set the new baud rate
        tab_map_[baud_tab_descriptor]->SetBaudRate(baud_rate_frame_type, baud_rate_frame, baud_rate_in_defaults);
    }

    QString success_message = "Default Settings Value Saved";
    iv.label_message->setText(success_message);
    iv.pcon->AddToLog(success_message);

    //Give the user the option to reboot the module after setting with defaults.
    QMessageBox msgBox;
    QAbstractButton * rebootButton = msgBox.addButton("Reboot Now", QMessageBox::YesRole);
    QAbstractButton * exitButton = msgBox.addButton("Do Not Reboot Now", QMessageBox::NoRole);

    msgBox.setWindowTitle("Defaults Set Successfully");

    QString text;

    if(!update_baud){
        text.append("Set values from default file successfully. We recommend that you restart your module"
                    " to ensure that all changes take effect. If you would like to restart your module now,"
                    " please select Reboot Now. Your module will disconnect from IQ Control Center.");
    }else{
        text.append("Set values from default file successfully. Since your module's UART Baud Rate has changed, and your "
                    "module has disconnected, we are unable to reboot your module through IQ Control Center. We highly "
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

    return;
  } else {
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.label_message->setText(error_message);
    return;
  }
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

    build_info.insert("Build information", version_information);

    json_array->append(build_info);
}

void MainWindow::write_parameters_to_file(QJsonArray * json_array, exportFileTypes exportStyle){
    //Now go through and get all of the values that are currently set on the control center/module

    //If we're connected, then we have a map of tabs (general, tuning, advanced, testing)
    //Tabs store all of our Frames (velocity kd, timeout, Voltage, etc.)
    //Each Frame stores the value that we got from the motor
    for (std::pair<std::string, std::shared_ptr<Tab>> tab : tab_map_) {
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
                  current_tab_json_object->insert("value", fc->value_);
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
          //using prepend to ensure that advanced ends up last
          json_array->prepend(top_level_tab_obj);
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

void MainWindow::on_generate_support_button_clicked(){
    //If we're connected then go get the values, otherwise just spit out a message to connect the motor
    if (iv.pcon->GetConnectionState() == 1) {
        write_user_support_file();
    }else{
        ui->header_error_label->setText("Please connect your module before attempting to generate your support file.");
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

    switch(fileExport){
        case exportFileTypes::SUPPORT_FILE:
            file_beginning.append("user_support_file_");
        break;

        case exportFileTypes::DEFAULTS_FILE:
            file_beginning.append("custom_defaults_");
        break;
    }

    //Let people pick a directory/name to save to/with, and save that path
    QString dir = QFileDialog::getSaveFileName(this, tr("Open Directory"),
                                                    "/home/" + file_beginning + ui->label_firmware_name->text() + ".json",
                                                    tr("json (*.json"));

    //Write to the json file
    QJsonDocument output_doc;
    output_doc.setArray(tab_array);

    QByteArray bytes = output_doc.toJson(QJsonDocument::Indented);

    //Above we filled the array with "entries" not "Entries," but to match our defaults format, we want it to say "Entries"
    QString text(bytes); // add to text string for easy string replace
    text.replace("entries", "Entries");

    QString path = dir;
    QFile file(path);

    file.setPermissions(path, QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteGroup |
                              QFileDevice::ReadOwner | QFileDevice::ReadUser | QFileDevice::ReadGroup);

    if( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        QTextStream iStream( &file );
        iStream.setCodec( "utf-8" );
        iStream << text;

        QMessageBox msgBox;
        msgBox.setWindowTitle("File Generated");

        QString text;

        switch(fileExport){
            case exportFileTypes::SUPPORT_FILE:
                text.append("Your support file has been succesfully generated at: " + path + ". "
                               "If you are not already in contact with a member of the Vertiq support team, please email this file "
                               "to support@vertiq.co with your name and complication, and we will respond as soon as possible.");

                msgBox.setStandardButtons(QMessageBox::Ok);

            break;

            case exportFileTypes::DEFAULTS_FILE:

                //If someone exports their defaults file, they'll probably want to use it again later on. So give them that option.
                text.append("Your module's current state has been saved in " + path + ". Would you like to add these defaults"
                            " to the Control Center now? If no, you will have to use the Import button to do so manually later.");

                msgBox.setStandardButtons(QMessageBox::No);
                msgBox.addButton(QMessageBox::Yes);

            break;
        }

        msgBox.setText(text);

        file.close();

        //If they click yes to import their export, then we'll have to directly import the file into the Defaults folder
        if(msgBox.exec() == QMessageBox::Yes){
            iv.pcon->AddToLog("Importing defaults file from current module state.");
            import_defaults_file_from_path(path);
        }

    } else {
       QString error_text("Unable to open output file location, please try again.");
       ui->header_error_label->setText(error_text);
       iv.pcon->AddToLog(error_text);
    }
}

void MainWindow::on_export_log_button_clicked(){
    //Grab the project log
    QFile currentLog(iv.pcon->path_to_log_file);

    QFileDialog dialog(this, tr("Open Directory"),
                       "/home/log.txt",
                       tr("txt (*.txt"));
    dialog.setFileMode(QFileDialog::AnyFile);

    //Let people pick a directory/name to save to/with, and save that path
    QString dir = dialog.getSaveFileName(this, tr("Open Directory"),
                                                    "/home/log.txt",
                                                    tr("text files (*.txt)"));

    //If the file already exists, kill it
    if(!(dir.isEmpty()) && QFile::exists(dir)){
        QFile::remove(dir);
    }

    if(!(dir.isEmpty())){

        //Pop up with where the log went (if successful)
        QMessageBox msgBox;
        msgBox.setWindowTitle("Exporting Log");
        QString text;

            //Copy the data from the project log to the user's desired location
            if(currentLog.copy(dir)){

                text.append("Your log file has been succesfully exported to: " + dir + ".");
            }else{
                text.append("Failed to export log.");
            }

            msgBox.setText(text);

            iv.pcon->AddToLog(text);

            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();

    }else{
        ui->header_error_label->setText("Unable to open output file location, please try again.");
    }
}

void MainWindow::on_clear_log_button_clicked(){
    ui->log_text_browser->clear();
}

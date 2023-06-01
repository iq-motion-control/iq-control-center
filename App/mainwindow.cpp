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

    Defaults *def = new Defaults(ui->default_box, "/Resources/Defaults/");

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
  if (iv.pcon->GetConnectionState() == 1 && !tab_map_.empty()) {
    std::map<std::string, double> default_value_map;
    for (uint8_t ii = 0; ii < defaults.size(); ++ii) {
      if (!defaults[ii]["Entries"].empty()) {
        default_value_map.clear();
        std::string tab_descriptor = defaults[ii]["descriptor"].asString() + ".json";
        Json::Value defaults_values = defaults[ii]["Entries"];
        for (uint8_t jj = 0; jj < defaults_values.size(); ++jj) {
          std::string value_descriptor = defaults_values[jj]["descriptor"].asString();
          double value = defaults_values[jj]["value"].asDouble();
          default_value_map[value_descriptor] = value;
        }
        std::map<std::string, std::shared_ptr<Tab>>::const_iterator it;
        it = tab_map_.find(tab_descriptor);
        if (it != tab_map_.end()) {
          tab_map_[tab_descriptor]->SaveDefaults(default_value_map);
          tab_map_[tab_descriptor]->CheckSavedValues();
        } else {
          QString error_message = "Wrong Default Settings Selected";
          iv.label_message->setText(error_message);
          return;
        }
      }
    }
    QString success_message = "Default Settings Value Saved";
    iv.label_message->setText(success_message);
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

void MainWindow::write_parameters_to_file(QJsonArray * json_array){
    //Now go through and get all of the values that are currently set on the control center/module

    //If we're connected, then we have a map of tabs (general, tuning, advanced, testing)
    //Tabs store all of our Frames (velocity kd, timeout, Voltage, etc.)
    //Each Frame stores the value that we got from the motor
    for (std::pair<std::string, std::shared_ptr<Tab>> tab : tab_map_) {
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
            current_tab_json_object->insert("descriptor", frame->first.c_str());
            tab_frame_array.append(*current_tab_json_object);
        }

        delete current_tab_json_object;
      }

      //Fill in with "entries" and "descriptors" so that the file is filled in the order matching our defaults files
      //If we fill "Entries" directly, it will be inserted above descriptor. While it wouldn't make a functional difference
      //It would be harder to read for a human
      top_level_tab_obj.insert("entries", tab_frame_array);
      top_level_tab_obj.insert("descriptor", tab.first.c_str());

      //Write to our output array
      json_array->append(top_level_tab_obj);

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
    write_parameters_to_file(&tab_array);

    //Let people pick a directory/name to save to/with, and save that path
    QString dir = QFileDialog::getSaveFileName(this, tr("Open Directory"),
                                                    "/home/user_support_file_" + ui->label_firmware_name->text() + ".json",
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
    if( file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        QTextStream iStream( &file );
        iStream.setCodec( "utf-8" );
        iStream << text;

        QMessageBox msgBox;
        msgBox.setWindowTitle("File Generated");

        QString text("Your support file has been succesfully generated at: " + path + ". "
                     "If you are not already in contact with a member of the Vertiq support team, please email this file "
                     "to support@vertiq.co with your name and complication, and we will respond as soon as possible.");
        msgBox.setText(text);

        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        file.close();
    }
    else
    {
       ui->header_error_label->setText("Unable to open output file location, please try again.");
    }
}

void MainWindow::on_generate_support_button_clicked(){
    //If we're connected then go get the values, otherwise just spit out a message to connect the motor
    if (iv.pcon->GetConnectionState() == 1) {
        write_user_support_file();
    }else{
        ui->header_error_label->setText("Please connect your module before attempting to generate your support file.");
    }
}

void MainWindow::on_export_log_button_clicked(){
    //Grab the project log
    QFile currentLog(iv.pcon->path_to_log_file);

    //Let people pick a directory/name to save to/with, and save that path
    QString dir = QFileDialog::getSaveFileName(this, tr("Open Directory"),
                                                    "/home/log.txt",
                                                    tr("txt (*.txt"));

    if(!(dir.isEmpty())){
        //Copy the data from the project log to the user's desired location
        currentLog.copy(dir);

        //Pop up with where the log went
        QMessageBox msgBox;
        msgBox.setWindowTitle("Log Exported");

        QString text("Your log file has been succesfully exported to: " + dir + ".");
        msgBox.setText(text);

        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }else{
        ui->header_error_label->setText("Unable to open output file location, please try again.");
    }
}


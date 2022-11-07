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

Firmware::Firmware(QProgressBar *flash_progress_bar, QPushButton *firmware_binary_button, QProgressBar *recover_progress_bar, QPushButton *recover_binary_button)
    : flash_progress_bar_(flash_progress_bar), firmware_binary_button_(firmware_binary_button),
      recover_progress_bar_(recover_progress_bar), recover_binary_button_(recover_binary_button){
  sys_map_ = ClientsFromJson(0, "system_control_client.json", clients_folder_path_);
}

void Firmware::SelectBinaryClicked() {
    const int FIRMWARE_TAB = 4;
    const int RECOVERY_TAB = 6;

    int currentTab = iv.pcon->GetCurrentTab();

    try {
        iv.label_message->clear();
        QFileDialog dialog;
        dialog.setFileMode(QFileDialog::ExistingFile);

        QPushButton * buttonInUse;

        QString desktop_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

        //Get the path to the file we want to use
        firmware_bin_path_ = QFileDialog::getOpenFileName(0, ("Select Firmware Binary"), desktop_dir,
                                                          tr("Binary (*.bin)"));
        //Pick which button we want to use
        if(currentTab == FIRMWARE_TAB){
            buttonInUse = firmware_binary_button_;
        }else if(currentTab == RECOVERY_TAB){
            buttonInUse = recover_binary_button_;
        }

        if (firmware_bin_path_.isEmpty()) {
          buttonInUse->setText("Select Firmware Binary (\".bin\")");
        } else {
          QFileInfo info(firmware_bin_path_);
          buttonInUse->setText(info.fileName());
        }

    }//try
        catch (const QString &e) {
        iv.label_message->setText(e);
    }//catch
}

void Firmware::FlashClicked() {
    const int FIRMWARE_TAB = 4;
    const int RECOVERY_TAB = 6;

    int currentTab = iv.pcon->GetCurrentTab();
    QString selected_port_name;

    if(currentTab == FIRMWARE_TAB){
        selected_port_name = iv.pcon->GetSelectedPortName();
    }else if(currentTab == RECOVERY_TAB){
        selected_port_name = iv.pcon->GetRecoveryPortName();
    }

    if (firmware_bin_path_.isEmpty()) {
        QString err_message = "No Firmware Binary Selected";
        iv.label_message->setText(err_message);
        return;
    }

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

        if(currentTab == FIRMWARE_TAB){
            Schmi::FlashLoader fl(&ser, &bin, &error, &flashBar);

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
        }

        if(currentTab == RECOVERY_TAB){
            Schmi::FlashLoader fl(&ser, &bin, &error, &recoverBar);

            fl.Init();
            bool init_usart = false;
            fl.Flash(init_usart);
            iv.pcon->ResetToTopPage();
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

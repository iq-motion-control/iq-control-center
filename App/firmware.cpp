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

void Firmware::SelectFirmwareClicked() {
  try {
    iv.label_message->clear();
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);

    QString desktop_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    firmware_bin_path_ = QFileDialog::getOpenFileName(0, ("Select Firmware Binary"), desktop_dir,
                                                      tr("Binary (*.bin)"));
    if (firmware_bin_path_.isEmpty()) {
      firmware_binary_button_->setText("Select Firmware Binary (\".bin\")");
    } else {
      QFileInfo info(firmware_bin_path_);
      firmware_binary_button_->setText(info.fileName());
    }

  } catch (const QString &e) {
    iv.label_message->setText(e);
  }
}

void Firmware::FlashClicked() {
  QString seletected_port_name = iv.pcon->GetSelectedPortName();

  if (firmware_bin_path_.isEmpty()) {
    QString err_message = "No Firmware Binary Selected";
    iv.label_message->setText(err_message);
    return;
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

  } catch (const QString &e) {
    iv.label_message->setText(e);
  }
}

void Firmware::SelectRecoveryClicked() {
  try {
    iv.label_message->clear();
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFile);

    QString desktop_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    recovery_bin_path_ = QFileDialog::getOpenFileName(0, ("Select Firmware Binary"), desktop_dir,
                                                      tr("Binary (*.bin)"));
    if (recovery_bin_path_.isEmpty()) {
      recover_binary_button_->setText("Select Firmware Binary (\".bin\")");
    } else {
      QFileInfo info(recovery_bin_path_);
      recover_binary_button_->setText(info.fileName());
    }

  } catch (const QString &e) {
    iv.label_message->setText(e);
  }
}

void Firmware::RecoverClicked(){
    QString seletected_port_name = iv.pcon->GetRecoveryPortName();

    if (recovery_bin_path_.isEmpty()) {
      QString err_message = "No Firmware Binary Selected";
      iv.label_message->setText(err_message);
      return;
    }

    //Do the same as a normal flash, but skip the part where we go into the bootloader, we're already there
    try {
      Schmi::ErrorHandlerStd error;
      Schmi::BinaryFileStd bin(recovery_bin_path_.toStdString());
      Schmi::QSerial ser(seletected_port_name);
      FlashLoadingBar bar(recover_progress_bar_);

      Schmi::FlashLoader fl(&ser, &bin, &error, &bar);

      fl.Init();

//      std::chrono::steady_clock::time_point time_start = std::chrono::steady_clock::now();
//      bool boot_mode = false;
//      while (!boot_mode) {
//        boot_mode = fl.InitUsart();
//        if (std::chrono::steady_clock::now() - time_start > std::chrono::milliseconds(10000)) {
//          throw QString("Could Not Init UART From Boot Mode");
//          break;
//        };
//      }

      bool init_usart = false;
      fl.Flash(init_usart);

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

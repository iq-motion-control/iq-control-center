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

#include "frame_spin_box.h"

FrameSpinBox::FrameSpinBox(QWidget *parent, Client *client,
                           std::pair<std::string, ClientEntryAbstract *> client_entry,
                           FrameVariables *fv,
                           bool using_custom_order, QString ordered_label, bool requires_restart)
    : Frame(parent, 2), client_(client), client_entry_(client_entry), requires_restart_(requires_restart) {
  nan_value_ = fv->spin_frame_.minimum;
  has_nan_ = fv->spin_frame_.nan;
  single_step_ = fv->spin_frame_.single_step;
  info_ = QString::fromStdString(fv->spin_frame_.info);
  setObjectName(QStringLiteral("frameSpinBox"));
  setGeometry(QRect(0, 0, 1029, 70));

  QSizePolicy size_policy = CreateSizePolicy();
  SetSettings(size_policy, style_sheet_);

  // creates a horizontal layout insisde the frame
  HorizontalLayout();

  // makes frame label
  // makes frame label
  if(using_custom_order){
    label_ = new QLabel(ordered_label, this);
  }else{
    label_ = new QLabel(QString((client_entry.first).c_str()), this);
  }

  SetLabel(label_, size_policy);
  horizontal_layout_->addWidget(label_);

  // creates spacer
  horizontal_spacer_ = new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  horizontal_layout_->addItem(horizontal_spacer_);

  // creates spin_box
  spin_box_ = new QDoubleSpinBox(this);
  SetSpinBox(size_policy, fv);
  QString unit = "   " + QString::fromStdString(fv->spin_frame_.unit);
  spin_box_->setToolTip(QString::fromStdString(fv->spin_frame_.unit));
  spin_box_->setSuffix(unit);
  if (has_nan_) {
    spin_box_->setSpecialValueText(tr("Nan"));
  }
  horizontal_layout_->addWidget(spin_box_);

  // creates push button default
  push_button_default_ = new QPushButton(this);
  SetPushButton(push_button_default_, size_policy, QString("pushButtonDefault"),
                push_button_default_icon_);
  push_button_default_->setToolTip(default_tip_);
  horizontal_layout_->addWidget(push_button_default_);

  // creates push buton save
  push_button_save_ = new QPushButton(this);
  SetPushButton(push_button_save_, size_policy, QString("pushButtonSave"),
                  push_button_save_icon_);
  push_button_save_->setToolTip(save_tip_);
  horizontal_layout_->addWidget(push_button_save_);

  // creates pushbutton info
  push_button_info_ = new QPushButton(this);
  SetPushButton(push_button_info_, size_policy, QString("pushButtonInfo"),
                push_button_info_icon_);
  horizontal_layout_->addWidget(push_button_info_);
  connect(push_button_info_, SIGNAL(clicked()), this, SLOT(ShowInfo()));

  //Initialize the value
  GetSavedValue();
  value_ = saved_value_;
}

void FrameSpinBox::ShowInfo() {
  QPoint globalPos = push_button_info_->mapToGlobal(push_button_info_->rect().topLeft());
  QWhatsThis::showText(globalPos, info_, push_button_info_);
}

void FrameSpinBox::SetSpinBox(QSizePolicy size_policy, FrameVariables *fv) {
  size_policy.setHeightForWidth(spin_box_->sizePolicy().hasHeightForWidth());
  QFont font;
  font.setFamily(QStringLiteral("Lato"));
  spin_box_->setObjectName(QStringLiteral("spin_box"));
  spin_box_->setSizePolicy(size_policy);
  spin_box_->setMinimumSize(QSize(122, 0));
  spin_box_->setMaximumSize(QSize(183, 16777215));
  spin_box_->setFont(font);
  spin_box_->setAutoFillBackground(false);
  spin_box_->setWrapping(false);
  spin_box_->setFrame(false);
  spin_box_->setReadOnly(false);
  spin_box_->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
  spin_box_->setAccelerated(false);
  spin_box_->setKeyboardTracking(true);
  spin_box_->setProperty("showGroupSeparator", QVariant(false));

  spin_box_->setRange(fv->spin_frame_.minimum, fv->spin_frame_.maximum);
  spin_box_->setSingleStep(fv->spin_frame_.single_step);
  spin_box_->setDecimals(fv->spin_frame_.decimal);
}

void FrameSpinBox::SaveValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      if (!SetVerifyEntrySave(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 5, 0.05f, value_)){
        QString error_msg("COULDN'T SAVE VALUE: " + QString(client_entry_.first.c_str()));
        iv.pcon->AddToLog(error_msg.toLower());
        throw QString(error_msg + " , please reconnect or try again");
      }

      iv.label_message->setText(QString("Value Saved Successfully"));
      iv.pcon->AddToLog("Set and saved value: " + QString(client_entry_.first.c_str()) + " = " + QString::number(value_));

      saved_value_ = value_;
      RemoveStarFromLabel();

      //If we need to restart when we change this parameter, then make the user restart
      if(requires_restart_){
        iv.pcon->RebootMotor();

        //Pop up a message saying what's going on
        //Give the user the option to reboot the module after setting with defaults.
        QMessageBox msgBox;
        msgBox.setWindowTitle("Reboot Required");
        QString text = "Setting this parameter requires a module reboot to take effect. We are rebooting your module now.";

        msgBox.setText(text);
        msgBox.exec();
      }

    } catch (const QString &e) {
      iv.label_message->setText(e);
    }
  } else {
    iv.pcon->AddToLog("No Motor Connected. Could not set " + QString((client_entry_.first).c_str()));
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.label_message->setText(error_message);
  }
}

void FrameSpinBox::GetSavedValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      if (!GetEntryReply(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 2, 0.05f, saved_value_)){
          std::string error_start = "";
          std::string error_string =   "COULDN'T GET SAVED VALUE: " + error_start + (client_entry_.first).c_str();
          iv.pcon->AddToLog(QString(error_string.c_str()).toLower());
          throw QString(error_string.c_str()) + " , please try again";
      }

      if (has_nan_ && std::isnan(saved_value_)) {
        saved_value_ = nan_value_;
      }
      spin_box_->setValue(saved_value_);

      iv.pcon->AddToLog(QString(client_entry_.first.c_str()) + " value gotten as: " + QString::number(saved_value_));

    } catch (const QString &e) {
      iv.label_message->setText(e);
    }
  } else {
    iv.pcon->AddToLog("No Motor Connected. Could not get " + QString((client_entry_.first).c_str()));
    QString error_message = "NO MOTOR CONNECTED, PLEASE CONNECT MOTOR";
    iv.label_message->setText(error_message);
  }
}

void FrameSpinBox::SpinBoxValueChanged(double spin_box_value) {
  if (!ApproxEqualStep(spin_box_value, saved_value_, single_step_)) {
    AddStarToLabel();
  } else {
    RemoveStarFromLabel();
  }

  if (has_nan_ && spin_box_value == nan_value_) {
    value_ = std::numeric_limits<double>::quiet_NaN();
  } else {
    value_ = spin_box_value;
  }
}

void FrameSpinBox::AddStarToLabel() {
  QString text = label_->text();
  if (text.back() != "*") {
    text.append(QString("*"));
    label_->setText(text);
  }
}

void FrameSpinBox::RemoveStarFromLabel() {
  QString text = label_->text();
  if (text.back() == "*") {
    text = text.left(text.length() - 1);
    label_->setText(text);
  }
}

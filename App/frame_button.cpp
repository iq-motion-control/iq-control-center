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

#include "frame_button.h"

FrameButton::FrameButton(QWidget *parent, Client *client,
                         std::pair<std::string, ClientEntryAbstract *> client_entry,
                         FrameVariables *fv, bool using_custom_order, QString ordered_label)
    : Frame(parent, 5), client_(client), client_entry_(client_entry) {
  info_ = QString::fromStdString(fv->button_frame_.info);
  // creates frame
  setObjectName(QStringLiteral("frameButton"));
  setGeometry(QRect(0, 10, 1029, 70));

  // creates size policy for the frame label
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

  // creates push buton save
  push_button_set_ = new QPushButton(this);
  SetPushButton(push_button_set_, size_policy, QString("pushButtonSet"), push_button_save_icon_);

  push_button_set_->setToolTip(save_tip_);
  horizontal_layout_->addWidget(push_button_set_);

  // creates pushbutton info
  push_button_info_ = new QPushButton(this);
  SetPushButton(push_button_info_, size_policy, QString("pushButtonInfo"),
                push_button_info_icon_);
  horizontal_layout_->addWidget(push_button_info_);
  connect(push_button_info_, SIGNAL(clicked()), this, SLOT(ShowInfo()));
}

void FrameButton::ShowInfo() {
  QPoint globalPos = push_button_info_->mapToGlobal(push_button_info_->rect().topLeft());
  QWhatsThis::showText(globalPos, info_, push_button_info_);
}

void FrameButton::SetValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      if (!client_->Set(*iv.pcon->GetQSerialInterface(), client_entry_.first)){

        QString error_msg("COULDN'T SET VALUE: " + QString(client_entry_.first.c_str()));

        iv.pcon->AddToLog(error_msg.toLower());

        throw QString(error_msg + " , please reconnect or try again");
      }

      iv.label_message->setText(QString("Value Saved Successfully"));
      iv.pcon->AddToLog("Set and saved value: " + QString(client_entry_.first.c_str()));

      iv.pcon->GetQSerialInterface()->SendNow();
    } catch (const QString &e) {
      iv.label_message->setText(e);
    }
  } else {
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.label_message->setText(error_message);
  }
}

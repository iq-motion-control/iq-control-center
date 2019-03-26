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

#include "frame_button.hpp"

FrameButton::FrameButton(QWidget *parent, Client* client, std::pair<std::string, ClientEntryAbstract*> client_entry, FrameVariables* fv) :
  Frame(parent,5),
  client_(client),
  client_entry_(client_entry)
{
  info_ = QString::fromStdString(fv->button_frame_.info);
  //creates frame
  setObjectName(QStringLiteral("frameButton"));
  setGeometry(QRect(0, 10, 1029, 70));

  //creates size policy for the frame label
  QSizePolicy size_policy =  CreateSizePolicy();
  SetSettings(size_policy, style_sheet_);

  //creates a horizontal layout insisde the frame
  HorizontalLayout();

  //makes frame label
  label_ = new QLabel(QString((client_entry.first).c_str()), this);
  SetLabel(label_, size_policy);
  horizontal_layout_->addWidget(label_);

  //creates spacer
  horizontal_spacer_ = new QSpacerItem(30, 20, QSizePolicy::Expanding,     QSizePolicy::Minimum);
  horizontal_layout_->addItem(horizontal_spacer_);

  //creates push buton save
  push_button_set_ = new QPushButton(this);
  SetPushButton(push_button_set_, size_policy, QString("pushButtonSet"), QString(":/res/save.png"));
  push_button_set_->setToolTip(save_tip_);
  horizontal_layout_->addWidget(push_button_set_);

  //creates pushbutton info
  push_button_info_ = new QPushButton(this);
  SetPushButton(push_button_info_, size_policy, QString("pushButtonInfo"),  QString(":/res/info_icon.png"));
  horizontal_layout_->addWidget(push_button_info_);
  connect(push_button_info_, SIGNAL(clicked()), this, SLOT(ShowInfo()));
}

void FrameButton::ShowInfo()
{
  QPoint globalPos = push_button_info_->mapToGlobal(push_button_info_->rect().topLeft());
  QWhatsThis::showText(globalPos,info_, push_button_info_);
}

void FrameButton::SetPushButton(QPushButton *push_button,QSizePolicy size_policy, QString push_button_name, QString icon_file_name )
{
  push_button->setObjectName(push_button_name);
  size_policy.setHeightForWidth(push_button->sizePolicy().hasHeightForWidth());
  push_button->setSizePolicy(size_policy);
  push_button->setMinimumSize(QSize(40, 30));
  push_button->setMaximumSize(QSize(40, 30));
  QIcon icon;
  icon.addFile(icon_file_name, QSize(), QIcon::Normal, QIcon::Off);
  push_button->setIcon(icon);
  push_button->setIconSize(QSize(20, 20));
  push_button->setFlat(true);
}


void FrameButton::SetValue()
{
  if(iv.pcon->GetConnectionState() == 1)
  {
    try
    {
      if(!client_->Set(*iv.pcon->GetQSerialInterface(), client_entry_.first))
        throw QString("COULDN'T SAVE VALUE: please reconnect or try again");
      iv.pcon->GetQSerialInterface()->SendNow();
    }
    catch(const QString &e)
    {
      iv.label_message->setText(e);
    }
  }
  else
  {
    QString error_message = "NO MOTOR CONNECTED, PLEASE CONNECT MOTOR";
    iv.label_message->setText(error_message);
  }
}

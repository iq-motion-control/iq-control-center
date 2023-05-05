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

#include "frame_switch.h"

FrameSwitch::FrameSwitch(QWidget *parent, Client* client, std::pair<std::string, ClientEntryAbstract*> client_entry, bool using_custom_order, QString ordered_label) :
  QWidget(parent),
  frame_(new Frame(parent)),
  client_(client),
  client_entry_(client_entry)
{
  //creates frame
  frame_->setObjectName(QStringLiteral("frameSwitch"));
  frame_->setGeometry(QRect(0, 10, 1029, 70));

  //creates size policy for the frame label
  QSizePolicy size_policy =  frame_->CreateSizePolicy();
  frame_->SetSettings(size_policy, style_sheet_);

  //creates a horizontal layout insisde the frame
  frame_->HorizontalLayout();

  //makes frame label
  if(using_custom_order){
    label_ = new QLabel(ordered_label, this);
  }else{
    label_ = new QLabel(QString((client_entry.first).c_str()), this);
  }

  frame_->SetLabel(label_, size_policy);
  frame_->horizontal_layout_->addWidget(label_);

  //creates spacer
  horizontal_spacer_ = new QSpacerItem(534, 20, QSizePolicy::Expanding,     QSizePolicy::Minimum);
  frame_->horizontal_layout_->addItem(horizontal_spacer_);

  switch_ = new Switch;
  SetSwitch(size_policy);
  frame_->horizontal_layout_->addWidget(switch_);

}

void FrameSwitch::SetSwitch(QSizePolicy size_policy)
{
  switch_->setObjectName(QStringLiteral("Switch"));
  size_policy.setHeightForWidth(switch_->sizePolicy().hasHeightForWidth());
  switch_->setSizePolicy(size_policy);
  switch_->setMinimumSize(QSize(60, 25));
  switch_->setMaximumSize(QSize(60, 25));
  switch_->setAutoFillBackground(false);
  switch_->setStyleSheet(QStringLiteral(""));
}

void FrameSwitch::SwitchClicked(bool switch_state)
{
  //TODO ask and handle switch ( what client entry actually needs that)
  QString message = "Switch activated";
  iv.label_message->setText(message);
}


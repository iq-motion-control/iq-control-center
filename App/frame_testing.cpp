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

#include "frame_testing.h"

FrameTesting::FrameTesting(QWidget *parent, Client* client, std::pair<std::string, ClientEntryAbstract*> client_entry, FrameVariables* fv) :
  Frame(parent, 4),
  client_(client),
  client_entry_(client_entry)
{

  setObjectName(QStringLiteral("FrameTesting"));
  setGeometry(QRect(0, 0, 1029, 70));

  QSizePolicy size_policy = CreateSizePolicy();
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

  //creates spin_box
  spin_box_ = new QDoubleSpinBox(this);
  SetSpinBox(size_policy, fv);
  horizontal_layout_->addWidget(spin_box_);
  QString unit = "   " + QString::fromStdString(fv->testing_frame_.unit);
  spin_box_->setToolTip(QString::fromStdString(fv->testing_frame_.unit));
  spin_box_->setSuffix(unit);

  //creates push buton set
  push_button_set_ = new QPushButton(this);
  SetPushButton(push_button_set_, size_policy, QString("pushButtonSave"), QString(":/res/save.png"));
  push_button_set_->setToolTip(set_tip_);
  horizontal_layout_->addWidget(push_button_set_);
}

void FrameTesting::SetPushButton(QPushButton *push_button,QSizePolicy size_policy, QString push_button_name, QString icon_file_name )
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

void FrameTesting::SetSpinBox(QSizePolicy size_policy, FrameVariables* fv)
{
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

  spin_box_->setRange(fv->testing_frame_.minimum, fv->testing_frame_.maximum);
  spin_box_->setSingleStep(fv->testing_frame_.single_step);
  spin_box_->setDecimals(fv->testing_frame_.decimal);
}

void FrameTesting::SetValue()
{
  if(iv.pcon->GetConnectionState() == 1)
  {
    try
    {
      client_->Set(*iv.pcon->GetQSerialInterface(), client_entry_.first, value_);
      iv.pcon->GetQSerialInterface()->SendNow();
      if(IsZero(value_, 0.00001))
      {
        QString success_message = label_->text()  + " SUCCESSFULLY SET TO " + QString::number(0) ;
        iv.label_message->setText(success_message);
      }
      else
      {
        QString success_message = label_->text()  + " SUCCESSFULLY SET TO " + QString::number(value_) ;
        iv.label_message->setText(success_message);
      }
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

void FrameTesting::SpinBoxValueChanged(double spin_box_value)
{
  value_ = spin_box_value;
}

bool FrameTesting::IsZero(double value, double threshold)
{
    return value >= -threshold && value <= threshold;
}

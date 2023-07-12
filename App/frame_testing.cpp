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

FrameTesting::FrameTesting(QWidget* parent, Client* client,
                           std::pair<std::string, ClientEntryAbstract*> client_entry,
                           FrameVariables* fv, bool using_custom_order, QString ordered_label)
    : Frame(parent, 4), client_(client), client_entry_(client_entry) {
  info_ = QString::fromStdString(fv->testing_frame_.info);
  default_value_ = fv->testing_frame_.default_value;
  value_ = default_value_;

  setObjectName(QStringLiteral("FrameTesting"));
  setGeometry(QRect(0, 0, 1029, 70));

  QSizePolicy size_policy = CreateSizePolicy();
  SetSettings(size_policy, style_sheet_);

  // creates a horizontal layout insisde the frame
  HorizontalLayout();

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
  horizontal_layout_->addWidget(spin_box_);
  QString unit = "   " + QString::fromStdString(fv->testing_frame_.unit);
  spin_box_->setToolTip(QString::fromStdString(fv->testing_frame_.unit));
  spin_box_->setSuffix(unit);
  spin_box_->setValue(default_value_);

  //Fills the space where a set button would be with empty space
  empty_slot_spacer_ = new QSpacerItem(46, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);
  horizontal_layout_->addItem(empty_slot_spacer_);

  // creates push buton set
  push_button_set_ = new QPushButton(this);
  SetPushButton(push_button_set_, size_policy, QString("pushButtonSave"),
                push_button_save_icon_);
  push_button_set_->setToolTip(set_tip_);
  horizontal_layout_->addWidget(push_button_set_);

  // creates pushbutton info
  push_button_info_ = new QPushButton(this);
  SetPushButton(push_button_info_, size_policy, QString("pushButtonInfo"),
                push_button_info_icon_);
  horizontal_layout_->addWidget(push_button_info_);
  connect(push_button_info_, SIGNAL(clicked()), this, SLOT(ShowInfo()));
}

void FrameTesting::ShowInfo() {
  QPoint globalPos = push_button_info_->mapToGlobal(push_button_info_->rect().topLeft());
  QWhatsThis::showText(globalPos, info_, push_button_info_);
}

void FrameTesting::SetSpinBox(QSizePolicy size_policy, FrameVariables* fv) {
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

  double maximimum = INT_MAX;
  double minimum = INT_MIN;

  if (!isnan(fv->testing_frame_.maximum)) {
    maximimum = fv->testing_frame_.maximum;
  }
  if (!isnan(fv->testing_frame_.minimum)) {
    minimum = fv->testing_frame_.minimum;
  }

  spin_box_->setRange(minimum, maximimum);
  spin_box_->setSingleStep(fv->testing_frame_.single_step);
  spin_box_->setDecimals(fv->testing_frame_.decimal);
}

void FrameTesting::SetValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      client_->Set(*iv.pcon->GetQSerialInterface(), client_entry_.first, value_);
      iv.pcon->GetQSerialInterface()->SendNow();
      if (IsZero(value_, 0.00001)) {
        QString success_message = label_->text() + " succesfully set to " + QString::number(0);
        iv.label_message->setText(success_message);
        iv.pcon->AddToLog(success_message);
      } else {
        QString success_message =
            label_->text() + " succesfully set to " + QString::number(value_);
        iv.label_message->setText(success_message);
        iv.pcon->AddToLog(success_message);
      }
    } catch (const QString& e) {
      iv.label_message->setText(e);
    }
  } else {
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.pcon->AddToLog("No Motor Connected. Could not set " + QString((client_entry_.first).c_str()));
    iv.label_message->setText(error_message);
  }
}

void FrameTesting::SpinBoxValueChanged(double spin_box_value) { value_ = spin_box_value; }

bool FrameTesting::IsZero(double value, double threshold) {
  return value >= -threshold && value <= threshold;
}

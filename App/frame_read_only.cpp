/*
    Copyright 2023 Vertiq   	jordan.leiber@vertiq.co

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

#include "frame_read_only.h"

FrameReadOnly::FrameReadOnly(QWidget *parent, Client *client,
                           std::pair<std::string, ClientEntryAbstract *> client_entry,
                           FrameVariables *fv, bool using_custom_order, QString ordered_label)
    : Frame(parent, 6), client_(client), client_entry_(client_entry) {
  nan_value_ = fv->read_only_frame_.minimum;
  has_nan_ = fv->read_only_frame_.nan;
  single_step_ = fv->read_only_frame_.single_step;
  info_ = QString::fromStdString(fv->read_only_frame_.info);
  setObjectName(QStringLiteral("frameReadOnly"));
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
  QString unit = "   " + QString::fromStdString(fv->read_only_frame_.unit);
  spin_box_->setToolTip(QString::fromStdString(fv->read_only_frame_.unit));
  spin_box_->setSuffix(unit);
  if (has_nan_) {
    spin_box_->setSpecialValueText(tr("Nan"));
  }
  spin_box_->setButtonSymbols(QAbstractSpinBox::NoButtons);
  horizontal_layout_->addWidget(spin_box_);

  // creates push button get
  push_button_get_ = new QPushButton(this);
  SetPushButton(push_button_get_, size_policy, QString("pushButtonGet"),
                push_button_default_icon_);
  push_button_get_->setToolTip(default_tip_);
  horizontal_layout_->addWidget(push_button_get_);

  //Fills the space where a set button would be with empty space
  empty_slot_spacer_ = new QSpacerItem(46, 20, QSizePolicy::Minimum, QSizePolicy::Minimum);
  horizontal_layout_->addItem(empty_slot_spacer_);

  // creates pushbutton info
  push_button_info_ = new QPushButton(this);
  SetPushButton(push_button_info_, size_policy, QString("pushButtonInfo"),
                push_button_info_icon_);
  horizontal_layout_->addWidget(push_button_info_);
  connect(push_button_info_, SIGNAL(clicked()), this, SLOT(ShowInfo()));
}

void FrameReadOnly::ShowInfo() {
  QPoint globalPos = push_button_info_->mapToGlobal(push_button_info_->rect().topLeft());
  QWhatsThis::showText(globalPos, info_, push_button_info_);
}

void FrameReadOnly::SetSpinBox(QSizePolicy size_policy, FrameVariables *fv) {
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
  spin_box_->setReadOnly(true);
  spin_box_->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
  spin_box_->setAccelerated(false);
  spin_box_->setKeyboardTracking(true);
  spin_box_->setProperty("showGroupSeparator", QVariant(false));

  spin_box_->setRange(fv->read_only_frame_.minimum, fv->read_only_frame_.maximum);
  spin_box_->setSingleStep(fv->read_only_frame_.single_step);
  spin_box_->setDecimals(fv->read_only_frame_.decimal);
}

void FrameReadOnly::GetSavedReadOnlyValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    if (!GetEntryReply(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 2, 0.05f, saved_value_)){
        std::string error_start = "";
        std::string error_string =   "COULDN'T GET SAVED VALUE: " + error_start + (client_entry_.first).c_str();
        iv.pcon->AddToLog(QString(error_string.c_str()).toLower());
    }
    if (has_nan_ && std::isnan(saved_value_)) {
      saved_value_ = nan_value_;
    }
    spin_box_->setValue(saved_value_);
  } else {
    iv.pcon->AddToLog("No Motor Connected. Could not get " + QString((client_entry_.first).c_str()));
    QString error_message = "NO MOTOR CONNECTED, PLEASE CONNECT MOTOR";
    iv.label_message->setText(error_message);
  }
}

void FrameReadOnly::SpinBoxValueChanged(double spin_box_value) {
  if (has_nan_ && spin_box_value == nan_value_) {
    value_ = std::numeric_limits<double>::quiet_NaN();
  } else {
    value_ = spin_box_value;
  }
}

void FrameReadOnly::AddStarToLabel() {
  QString text = label_->text();
  if (text.back() != "*") {
    text.append(QString("*"));
    label_->setText(text);
  }
}

void FrameReadOnly::RemoveStarFromLabel() {
  QString text = label_->text();
  if (text.back() == "*") {
    text = text.left(text.length() - 1);
    label_->setText(text);
  }
}

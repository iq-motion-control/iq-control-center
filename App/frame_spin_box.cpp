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
                           bool using_custom_order, QString ordered_label)
    : Frame(parent, 2), client_(client), client_entry_(client_entry) {
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
                QString(":/res/default_icon.png"));
  push_button_default_->setToolTip(default_tip_);
  horizontal_layout_->addWidget(push_button_default_);

  // creates push buton save
  push_button_save_ = new QPushButton(this);
  SetPushButton(push_button_save_, size_policy, QString("pushButtonSave"),
                QString(":/res/save.png"));
  push_button_save_->setToolTip(save_tip_);
  horizontal_layout_->addWidget(push_button_save_);

  // creates pushbutton info
  push_button_info_ = new QPushButton(this);
  SetPushButton(push_button_info_, size_policy, QString("pushButtonInfo"),
                QString(":/res/info_icon.png"));
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

void FrameSpinBox::SetPushButton(QPushButton *push_button, QSizePolicy size_policy,
                                 QString push_button_name, QString icon_file_name) {
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
      if (!SetVerifyEntrySave(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 5,
                              0.05f, value_))
        throw QString("COULDN'T SAVE VALUE: please reconnect or try again");

      iv.label_message->setText(QString("Value Saved Successfully"));
      iv.label_message->setText(QString("Value Saved Successfully"));
      saved_value_ = value_;
      RemoveStarFromLabel();
    } catch (const QString &e) {
      iv.label_message->setText(e);
    }
  } else {
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.label_message->setText(error_message);
  }
}

void FrameSpinBox::GetSavedValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      if (!GetEntryReply(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 2, 0.05f,
                         saved_value_))
        throw QString("COULDN'T GET SAVED VALUE: please reconnect or try again");
      if (has_nan_ && std::isnan(saved_value_)) {
        saved_value_ = nan_value_;
      }
      spin_box_->setValue(saved_value_);
    } catch (const QString &e) {
      iv.label_message->setText(e);
    }
  } else {
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

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

#include "frame_combo.h"

FrameCombo::FrameCombo(QWidget *parent, Client *client,
                       std::pair<std::string, ClientEntryAbstract *> client_entry,
                       FrameVariables *fv,
                       bool using_custom_order, QString ordered_label)

    : Frame(parent, 1), client_(client), client_entry_(client_entry) {
  info_ = QString::fromStdString(fv->combo_frame_.info);

  // creates frame
  setObjectName(QStringLiteral("frameCombo"));
  setGeometry(QRect(0, 10, 1029, 70));

  // creates size policy for the frame label
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

  // creates combo_box
  combo_box_ = new QComboBox(this);
  SetBox(size_policy, fv);
  horizontal_layout_->addWidget(combo_box_);

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
  value_ = index_value_[combo_box_->currentIndex()];
}

void FrameCombo::ShowInfo() {
  QPoint globalPos = push_button_info_->mapToGlobal(push_button_info_->rect().topLeft());
  QWhatsThis::showText(globalPos, info_, push_button_info_);
}

void FrameCombo::SetBox(QSizePolicy size_policy, FrameVariables *fv) {
  size_policy.setHeightForWidth(combo_box_->sizePolicy().hasHeightForWidth());

  size_t item_num = fv->combo_frame_.list_names.size();
  for (size_t ii = 0; ii < item_num; ++ii) {
    QString item_name = QString::fromStdString(fv->combo_frame_.list_names[ii]);
    combo_box_->addItem(item_name);
    index_value_[ii] = fv->combo_frame_.list_values[ii];
  }

  combo_box_->setObjectName(QStringLiteral("comboBox"));
  combo_box_->setSizePolicy(size_policy);
  combo_box_->setMinimumSize(QSize(164, 0));
  combo_box_->setMaximumSize(QSize(150, 16777215));
  QFont font;
  font.setFamily(QStringLiteral("Lato"));
  combo_box_->setFont(font);
  combo_box_->setContextMenuPolicy(Qt::DefaultContextMenu);
  combo_box_->setLayoutDirection(Qt::LeftToRight);
  combo_box_->setAutoFillBackground(false);
  combo_box_->setStyleSheet(QStringLiteral(""));
  combo_box_->setEditable(false);

  //This is meant to dictate what to do if the content of the combox box changes, but the content of our boxes doesn't change,
  //They aren't editable and we don't change it after initial creation. There is no visual difference from setting this size adjust policy
  //that I can tell. But it does increase RAM usage by over 300 MB, so we are better off without. Leaving it commented out for historical refrence.
  //combo_box_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

void FrameCombo::SaveValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      if (!SetVerifyEntrySave(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 5, 0.05f, value_)){
          QString error_str("COULDN'T SAVE VALUE: " + QString(client_entry_.first.c_str()));
          iv.pcon->AddToLog(error_str.toLower());
          throw QString(error_str + " , please reconnect or try again");
      }

      iv.label_message->setText(QString("Value Saved Successfully"));
      iv.pcon->AddToLog("set and saved value: " + QString(client_entry_.first.c_str()) + " = " + QString::number(value_));

      saved_value_ = value_;
      RemoveStarFromLabel();
    } catch (const QString &e) {
      iv.label_message->setText(e);
    }
  } else {
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.pcon->AddToLog("No Motor Connected. Could not set: " + QString(client_entry_.first.c_str()) + " = " + QString::number(value_));
    iv.label_message->setText(error_message);
  }
}

void FrameCombo::GetSavedValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    if (!GetEntryReply(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 5, 0.05f,
                       saved_value_)){
      std::string error_start = "";
      std::string error_string =   "COULDN'T GET SAVED VALUE: " + error_start + (client_entry_.first).c_str();
      iv.pcon->AddToLog(QString(error_string.c_str()).toLower());
    }

    int key = 0;
    for (auto &i : index_value_) {
      if (i.second == saved_value_) {
        key = i.first;
        break;  // to stop searching
      }
    }
    int combo_box_index = key;
    combo_box_->setCurrentIndex(combo_box_index);

    iv.pcon->AddToLog(QString(client_entry_.first.c_str()) + " value index gotten as: " + QString::number(combo_box_index));
  } else {
    iv.pcon->AddToLog("No Motor Connected. Could not get " + QString((client_entry_.first).c_str()));
    QString error_message = "No Motor Connected, Please Connect Motor";
    iv.label_message->setText(error_message);
  }
}

void FrameCombo::ComboBoxIndexChange(int index) {
  value_ = index_value_[index];

  if (value_ != saved_value_) {
    AddStarToLabel();
  } else {
    RemoveStarFromLabel();
  }
}

void FrameCombo::AddStarToLabel() {
  QString text = label_->text();
  if (text.back() != "*") {
    text.append(QString("*"));
    label_->setText(text);
  }
}

void FrameCombo::RemoveStarFromLabel() {
  QString text = label_->text();
  if (text.back() == "*") {
    text = text.left(text.length() - 1);
    label_->setText(text);
  }
}

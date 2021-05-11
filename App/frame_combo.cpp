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
                       FrameVariables *fv)
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
  label_ = new QLabel(QString((client_entry.first).c_str()), this);
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
}

void FrameCombo::ShowInfo() {
  QPoint globalPos = push_button_info_->mapToGlobal(push_button_info_->rect().topLeft());
  QWhatsThis::showText(globalPos, info_, push_button_info_);
}

void FrameCombo::SetPushButton(QPushButton *push_button, QSizePolicy size_policy,
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
  combo_box_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
}

void FrameCombo::SaveValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      if (!SetVerifyEntrySave(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 5,
                              0.05f, value_))
        throw QString("COULDN'T SAVE VALUE: please reconnect or try again");
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

void FrameCombo::GetSavedValue() {
  if (iv.pcon->GetConnectionState() == 1) {
    try {
      if (!GetEntryReply(*iv.pcon->GetQSerialInterface(), client_, client_entry_.first, 5, 0.05f,
                         saved_value_))
        throw QString("COULDN'T GET SAVED VALUE: please reconnect or try again");

      int key = 0;
      for (auto &i : index_value_) {
        if (i.second == saved_value_) {
          key = i.first;
          break;  // to stop searching
        }
      }
      int combo_box_index = key;
      combo_box_->setCurrentIndex(combo_box_index);
    } catch (const QString &e) {
      iv.label_message->setText(e);
    }
  } else {
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

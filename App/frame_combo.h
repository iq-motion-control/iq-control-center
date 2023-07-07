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

#ifndef FRAME_COMBO_H
#define FRAME_COMBO_H

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QWhatsThis>

#include "main.h"

#include "IQ_api/client.hpp"

#include "frame.h"
#include "frame_variables.h"
#include "common_icon_creation.h"

class FrameCombo : public Frame {
  Q_OBJECT
 public:
  FrameCombo(QWidget *parent, Client *client,
             std::pair<std::string, ClientEntryAbstract *> client_entry, FrameVariables *fv, bool using_custom_order, QString ordered_label);

  QComboBox *combo_box_;
  QPushButton *push_button_default_;
  QPushButton *push_button_save_;
  QPushButton *push_button_info_;
  int value_;

 public slots:
  void SaveValue();
  void GetSavedValue();
  void ComboBoxIndexChange(int index);
  void ShowInfo();

 private:
  void SetBox(QSizePolicy size_policy, FrameVariables *fv);

  void AddStarToLabel();
  void RemoveStarFromLabel();

  Client *client_;
  std::map<int, int> index_value_;
  std::pair<std::string, ClientEntryAbstract *> client_entry_;
  int saved_value_;
  QString info_;
  QSpacerItem *horizontal_spacer_;
  QLabel *label_;
  QString save_tip_ = "Saves value to motor";
  QString default_tip_ = "Check saved value on motor";
  QLatin1String style_sheet_ = QLatin1String(
      "QFrame#frameCombo {\n"
      "	border: 0px;\n"
      "	border-bottom: 1px solid rgb(50, 50, 50);\n"
      "}\n"
      "\n"
      "QComboBox {\n"
      "     color:  rgb(175, 175, 175);\n"
      "	border-bottom: 1px solid rgb(175, 175, 175);\n"
      "	border-radius: 0px;\n"
      "    padding: 3px 20px 3px 3px;\n"
      "    min-width: 10em;\n"
      "	selection-background-color: transparent;\n"
      "    selection-color: rgb(1, 150, 250);\n"
      "	background: transparent;\n"
      "	padding: 2px\n"
      "}\n"
      "\n"
      "QComboBox::drop-down {\n"
      "    width: 20px; /*changes size of arrow icon section and also changes size of drop down "
      "menu*/\n"
      "    border-left-width: 0px;  /*adjusts border of arrow icon section*/\n"
      "    border-left-color: blue;\n"
      "    border-left-style: solid;\n"
      "    border-top-right-radius: 3px; /* same radius as the QComboBox */\n"
      "    border-bottom-right-radius: 3px;\n"
      "	background: transparent;\n"
      "	image:url(:/res/down_arrow_icon.png);"
      "}\n"
      "\n"
      "QComboBox::hover {\n"
      "    border: 1px solid transparent;\n"
      "	border-bottom: 1px solid rgb(200, 200, 200);\n"
      "}\n"
      ""
      "\n"
      "QComboBox::on, QComboBox::off {\n"
      "    color: rgb(60, 60, 60)\n"
      "}"
      "QPushButton {\n"
      "	background: transparent;\n"
      "	height: 30px;\n"
      "	border: 0px solid rgb(175, 175, 175);\n"
      "	border-radius: 8px\n"
      "}\n"
      "\n"
      "QPushButton::hover, QPushButton:pressed {\n"
      "     background: rgb(60, 60, 60);\n"
      "}\n");
};

#endif  // FRAME_COMBO_H

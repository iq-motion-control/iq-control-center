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

#ifndef FRAME_SWITCH_H
#define FRAME_SWITCH_H

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>

#include "main.h"

#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"

#include "frame_variables.h"
#include "frame.h"
#include "switch.h"


class FrameSwitch: public QWidget
{
    Q_OBJECT
  public:
    FrameSwitch(QWidget *parent, Client* client, std::pair<std::string, ClientEntryAbstract*> client_entry);

    Frame *frame_;
    Switch *switch_;

  public slots:
    void SwitchClicked(bool switch_state);

  private:

    void SetSwitch(QSizePolicy size_policy);

    Client *client_;
    std::pair<std::string, ClientEntryAbstract*> client_entry_;
    QSpacerItem *horizontal_spacer_;
    QLabel *label_;
    QLatin1String style_sheet_ = QLatin1String(
          "QFrame#frameSwitch {\n" "	border: 0px;\n"
          "	border-bottom: 1px solid rgb(50, 50, 50);\n"
          "}\n"
          "\n"
          "QComboBox {\n"
          "	border-bottom: 1px solid rgb(175, 175, 175);    \n"
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
          "    width: 20px; /*changes size of arrow icon section and also changes size of drop down menu*/\n"
          "    border-left-width: 0px;  /*adjusts border of arrow icon section*/\n"
          "    border-left-color: blue;\n"
          "    border-left-style: solid;\n"
          "    border-top-right-radius: 3px; /* same radius as the QComboBox */\n"
          "    border-bottom-right-radius: 3px;\n"
          "	background: transparent;\n"
          "	image: url(/Users/iqinetics/Desktop/IQ_GUI/res/down_arrow_icon.png);\n"
          "}\n"
          "\n"
          "QComboBox:hover {\n"
          "    border: 1px solid transparent;\n"
          "	border-bottom: 1px solid rgb(200, 200, 200);\n"
          "}\n"
          "\n"
          "Q"
          "ComboBox:on, QComboBox:off {\n"
          "    color: rgb(60, 60, 60)\n"
          "}");

};

#endif // FRAME_SWITCH_H

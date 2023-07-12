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

#ifndef FRAME_BUTTON_HPP
#define FRAME_BUTTON_HPP


#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QObject>
#include <QWhatsThis>
#include <QObject>

#include "main.h"

#include "IQ_api/client.hpp"
#include "frame_variables.h"
#include "frame.h"
#include "common_icon_creation.h"

class FrameButton : public Frame
{
    Q_OBJECT
  public:
    FrameButton(QWidget *parent, Client* client, std::pair<std::string, ClientEntryAbstract*> client_entry, FrameVariables* fv, bool using_custom_order, QString ordered_label);

    QPushButton *push_button_set_;
    QPushButton *push_button_info_;


  public slots:
    void SetValue();
    void ShowInfo();

  private:

    Client *client_;
    std::pair<std::string, ClientEntryAbstract*> client_entry_;

    QString info_;
    QSpacerItem *horizontal_spacer_;
    QLabel *label_;
    QString save_tip_ = "Sets value to motor";
    QString default_tip_ = "Check saved value on motor";
    QLatin1String style_sheet_ = QLatin1String(
          "QFrame#frameButton {\n"
          "	border: 0px;\n"
          "	border-bottom: 1px solid rgb(50, 50, 50);\n"
          "}\n"
          "\n"
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

#endif // FRAME_BUTTON_HPP

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

#ifndef FRAME_READ_ONLY_H
#define FRAME_READ_ONLY_H

#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QToolTip>
#include <QWhatsThis>

#include "main.h"

#include "IQ_api/client.hpp"
#include "frame.h"
#include "frame_variables.h"

class FrameReadOnly: public Frame
{
    Q_OBJECT
  public:
    FrameReadOnly(QWidget *parent, Client* client, std::pair<std::string, ClientEntryAbstract*> client_entry, FrameVariables* fv, bool using_custom_order, QString ordered_label);
    QSpacerItem *empty_slot_spacer_;
    QPushButton *push_button_get_;
    QDoubleSpinBox *spin_box_;
    QPushButton *push_button_info_;
    double value_;

  public slots:
    void GetSavedReadOnlyValue();
    void SpinBoxValueChanged(double spin_box_value);
    void ShowInfo();

  private:

    void SetPushButton(QPushButton *push_button,QSizePolicy size_policy, QString push_button_name, QString icon_file_name );

    void SetSpinBox(QSizePolicy size_policy, FrameVariables* fv);

    void AddStarToLabel();
    void RemoveStarFromLabel();

    Client *client_;
    std::map<int, int> index_value_ ;
    double saved_value_ = std::numeric_limits<double>::quiet_NaN();
    double nan_value_;
    bool has_nan_ = 0;
    float single_step_;
    QString info_;
    std::pair<std::string, ClientEntryAbstract*> client_entry_;
    QSpacerItem *horizontal_spacer_;
    QLabel *label_;
    QString default_tip_ = "Check saved value on motor";
    QLatin1String style_sheet_ = QLatin1String(
          "QFrame#frameReadOnly {\n"
          "	border: 0px;\n"
          "	border-bottom: 1px solid rgb(50, 50, 50);\n"
          "}\n"
          "\n"
          "QDoubleSpinBox {\n"
          "	border: 0px solid rgb(175, 175, 175);    \n"
          "	border-radius: 0px;\n"
          "    padding: 5px 5px 5px 5px;\n"
          "    min-width: 132px;\n"
          "	color: rgb(175, 175, 175);\n"
          "	background: rgb(60, 60, 60);\n"
          "}\n"
          "\n"
          "QDoubleSpinBox::up-button {\n"
          "	image: url(:/res/up_arrow_icon.png);"
          "	background: transparent;\n"
          "	width: 20px;\n"
          "}\n"
          "\n"
          "QDoubleSpinBox::up-button:hover {\n"
          "    background: rgb(50, 50, 50)\n"
          "}\n"
          "\n"
          "QDoubleSpinBox::down-button {\n"
          "	image: url(:/res/down_arrow_icon.png);"
          "	background: transparent;\n"
          "	width: 20px;\n"
          "}\n"
          "\n"
          "QDoubleSpinBox::down-button:hover {\n"
          "    background: rgb(50, 50, 50)\n"
          "}\n"
          "\n"
          "QPushButton {\n"
          "	background: transparent;\n"
          "	height: 30px;\n"
          "	border: 0px solid rgb(175, 175, 175);\n"
          "	border-radius: 8px\n"
          "}\n"
          "\n"
          "QPushButton:hover, QPushButton:pressed {\n"
          "	b"
          "ackground: rgb(60, 60, 60);\n"
          "}\n"
          "	\n"
          "\n"
          "/*\n"
          "image:  url(:/res/down_arrow_icon.png);\n"
          " url(:/res/up_arrow_icon.png);");
};


#endif  // FRAME_READ_ONLY_H

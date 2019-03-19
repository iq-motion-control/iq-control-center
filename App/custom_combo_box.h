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

#ifndef CUSTOM_COMBO_BOX_H
#define CUSTOM_COMBO_BOX_H

#include <QComboBox>
#include <iostream>

class CustomComboBox : public QComboBox
{
    Q_OBJECT
  public:
    explicit CustomComboBox(QWidget *parent = nullptr);
    void showPopup();

  signals:
    void CustomComboBoxSelected();
};

#endif // CUSTOM_COMBO_BOX_H

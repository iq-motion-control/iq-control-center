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

#ifndef CUSTOMDOUBLESPINBOX_H
#define CUSTOMDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QWidget>
#include <iostream>

#define NAN_VALUE 32767

class CustomDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    CustomDoubleSpinBox(QWidget *parent = 0);
    virtual ~CustomDoubleSpinBox() throw() {}


    double valueFromText(const QString &text) const;
    QString textFromValue(int value) const;
    QValidator::State validate(QString &input, int &pos);
};

#endif // CUSTOMDOUBLESPINBOX_H


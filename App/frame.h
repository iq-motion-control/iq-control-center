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

#ifndef FRAME_H
#define FRAME_H

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>


class Frame : public QFrame
{
    Q_OBJECT
  public:
    explicit Frame(QWidget *parent = nullptr, int frame_type = -1);

    void SetLabel(QLabel *label, QSizePolicy size_policy);
    void SetSettings(QSizePolicy sizePolicy,  QLatin1String style_sheet);
    QSizePolicy CreateSizePolicy();
    void HorizontalLayout();

    void SetPushButton(QPushButton *push_button, QSizePolicy size_policy, QString push_button_name,
                       QString icon_file_name);
    void SetPushButton(QPushButton *push_button, QSizePolicy size_policy,
                                   QString push_button_name, QIcon icon_to_use);

    QHBoxLayout *horizontal_layout_;

    int frame_type_ = -1;

  signals:

  public slots:
};

#endif // FRAME_H

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

#include "frame.h"

Frame::Frame(QWidget *parent, int frame_type) :
  QFrame(parent),
  frame_type_(frame_type)
{
}

void Frame::SetLabel(QLabel *label, QSizePolicy size_policy)
{
  label->setObjectName(QStringLiteral("label_setting"));
  size_policy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
  label->setSizePolicy(size_policy);
  QFont font;
  font.setFamily(QStringLiteral("Lato"));
  font.setPointSize(20);
  label->setFont(font);
}

void Frame::HorizontalLayout()
{
  horizontal_layout_ = new QHBoxLayout(this);
  horizontal_layout_->setSpacing(6);
  horizontal_layout_->setContentsMargins(11, 11, 11, 11);
  horizontal_layout_->setObjectName(QStringLiteral("horizontalLayout"));
}

void Frame::SetSettings(QSizePolicy size_policy, QLatin1String style_sheet)
{
  size_policy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
  this->setSizePolicy(size_policy);
  this->setMinimumSize(QSize(0, 70));
  this->setLayoutDirection(Qt::LeftToRight);
  this->setAutoFillBackground(false);
  this->setStyleSheet(style_sheet);
  this->setFrameShape(QFrame::StyledPanel);
  this->setFrameShadow(QFrame::Raised);
}

QSizePolicy Frame::CreateSizePolicy()
{
  QSizePolicy size_policy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  size_policy.setHorizontalStretch(0);
  size_policy.setVerticalStretch(0);
  return size_policy;
}

//Left around for backwards compatibility. Should be avoided due to increased RAM usage.
void Frame::SetPushButton(QPushButton *push_button, QSizePolicy size_policy,
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

void Frame::SetPushButton(QPushButton *push_button, QSizePolicy size_policy,
                               QString push_button_name, QIcon icon_to_use) {
  push_button->setObjectName(push_button_name);
  size_policy.setHeightForWidth(push_button->sizePolicy().hasHeightForWidth());
  push_button->setSizePolicy(size_policy);
  push_button->setMinimumSize(QSize(40, 30));
  push_button->setMaximumSize(QSize(40, 30));
  push_button->setIcon(icon_to_use);
  push_button->setIconSize(QSize(20, 20));
  push_button->setFlat(true);
}

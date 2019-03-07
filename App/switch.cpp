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

#include "switch.h"

Switch::Switch(QWidget *parent) : QAbstractButton(parent),
  _switch(false),
  _opacity(0.000),
  _height(16),

  _margin(3),
  _thumb("#afafaf"),
  _anim(new QPropertyAnimation(this, "offset", this))
{
  setOffset(_height / 2);
  _y = _height / 2;
  setBrush(QColor("#0196fa"));
}

Switch::Switch(const QBrush &brush, QWidget *parent) : QAbstractButton(parent),
  _switch(false),
  _opacity(0.000),
  _height(16),
  _margin(3),
  _thumb("red"),
  _anim(new QPropertyAnimation(this, "offset", this))
{
  setOffset(_height / 2);
  _y = _height / 2;
  setBrush(brush);
}

void Switch::paintEvent(QPaintEvent *e)
{
  QPainter p(this);
  p.setPen(Qt::NoPen);
  if (isEnabled())
  {
    p.setBrush(_switch ? brush() : Qt::black);
    p.setOpacity(_switch ? .5 : .2);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawRoundedRect(QRect(_margin, _margin, width() - 2 * _margin, height() - 2 * _margin), 8.0, 8.0);
    p.setBrush(_thumb);
    p.setOpacity(1.0);
    p.drawEllipse(QRectF(offset() - (_height / 2), _y - (_height / 2), height(), height()));
  }
  else
  {
    p.setBrush(Qt::blue);
    p.setOpacity(0.12);
    p.drawRoundedRect(QRect(_margin, _margin, width() - 2 * _margin, height() - 2 * _margin), 8.0, 8.0);
    p.setOpacity(1.0);
    p.setBrush(QColor("#0196fa"));
    p.drawEllipse(QRectF(offset() - (_height / 2), _y - (_height / 2), height(), height()));
  }
}

void Switch::mouseReleaseEvent(QMouseEvent *e)
{
  if (e->button() & Qt::LeftButton)
  {
    _switch = _switch ? false : true;
    _thumb = _switch ? _brush : QBrush("#afafaf");
    if (_switch)
    {
      _anim->setStartValue(_height / 2);
      _anim->setEndValue(width() - _height / .9);
      _anim->setDuration(120);
      _anim->start();
    }
    else
    {
      _anim->setStartValue(offset());
      _anim->setEndValue(_height / 2);
      _anim->setDuration(120);
      _anim->start();
    }
  }
  QAbstractButton::mouseReleaseEvent(e);
  emit SwitchChanged(_switch);
}

void Switch::enterEvent(QEvent *e)
{
  setCursor(Qt::PointingHandCursor);
  QAbstractButton::enterEvent(e);
}

QSize Switch::sizeHint() const
{
  return QSize(5 * (_height + _margin), _height + 2 * _margin);
}

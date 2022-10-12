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

#include "mainwindow.h"
#include <QApplication>
#include "switch.h"
#include "main.h"
#include <QDebug>

//initializes global struct
InterfaceVariables iv;

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  //Create an instance of a MainWindow object. This is what really runs the program 
  //See mainwindow.cpp for more details
  MainWindow w; 
  w.show(); //Turn on the application
  return a.exec();
}

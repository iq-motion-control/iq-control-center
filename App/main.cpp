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
//#include <updater.h>
#include <updatecontroller.h>

//initializes global struct
InterfaceVariables iv;

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
//  MainWindow w;
//  w.show();
  //create the updater with the application as parent -> will live long enough start the tool on exit
//  QtAutoUpdater::Updater *updater = new QtAutoUpdater::Updater("C:/Qt/MaintenanceTool", &a);//.exe is automatically added

//  QObject::connect(updater, &QtAutoUpdater::Updater::checkUpdatesDone, [updater](bool hasUpdate, bool hasError) {
//    qDebug() << "Has updates:" << hasUpdate << "\nHas errors:" << hasError;
////    if(hasUpdate) {
////      //As soon as the application quits, the maintenancetool will be started in update mode
//////      updater->runUpdaterOnExit();
////      qDebug() << "Update info:" << updater->updateInfo();
////    }
////    //Quit the application
//////    qApp->quit();
//  });

//  QApplication::setQuitOnLastWindowClosed(false);
//  //since there is no parent window, all dialogs will be top-level windows
//  QtAutoUpdater::UpdateController *controller = new QtAutoUpdater::UpdateController("C:/Qt/MaintenanceTool", &a);//.exe is automatically added
////  QObject::connect(controller, &QtAutoUpdater::UpdateController::runningChanged, [controller](bool running) {
////          qDebug() << "Running changed:" << running;
////          //quit the application as soon as the updating finished
////          if(!running)
////              qApp->quit();
////      });
//  //start the update check -> AskLevel to give the user maximum control
//  controller->start(QtAutoUpdater::UpdateController::AskLevel);

  MainWindow w;
  w.show();
//  updater->checkForUpdates();
  return a.exec();
}

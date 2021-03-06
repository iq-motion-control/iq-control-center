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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QBoxLayout>
#include <QMainWindow>

#include "defaults.h"
#include "firmware.h"
#include "home.h"
#include "tab.h"
#include "tab_populator.h"

#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"
#include "custom_combo_box.h"

#include <unistd.h>
#include <map>
#include <string>
#include <vector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  std::map<std::string, std::shared_ptr<Tab>> tab_map_;

  QProcess *process;

  void AutoCheckUpdate();

 private slots:
  void on_pushButton_home_clicked();

  void on_pushButton_general_clicked();

  void on_pushButton_tuning_clicked();

  void on_pushButton_testing_clicked();

  void on_pushButton_advanced_clicked();

  void on_pushButton_firmware_clicked();

  void ShowMotorSavedValues();

  void updater();

  void readOutput();

  void ClearTabs();

  void SetDefaults(Json::Value defaults);

 private:
  Ui::MainWindow *ui;
  //    QtAutoUpdater::UpdateController *controller;
  //    QtAutoUpdater::UpdateButton *updateButton;

  TabPopulator *tab_populator;

};

#endif  // MAINWINDOW_H

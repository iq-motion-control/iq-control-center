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
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->stackedWidget->setCurrentIndex(0);
  ui->pushButton_home->setChecked(1);

  Updater *update = new Updater(ui,parent);
  update->ConnectUpdater();

  QString gui_version = QString::number(MAJOR) + "." + QString::number(MINOR) + "." + QString::number(PATCH);
  ui->label_gui_version_value->setText(gui_version);
  try
  {
    iv.pcon = new PortConnection(ui);
    iv.label_message = ui->header_error_label;

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), iv.pcon, SLOT(TimerTimeout()));
    timer->start(1000);

    connect(ui->header_combo_box, SIGNAL(CustomComboBoxSelected()), iv.pcon, SLOT(FindPorts()));
    connect(ui->header_combo_box, QOverload<int>::of(&QComboBox::activated), iv.pcon, &PortConnection::PortComboBoxIndexChanged);

    connect(ui->header_connect_button, SIGNAL(clicked()), iv.pcon, SLOT(ConnectMotor()));
    iv.pcon->FindPorts();

    connect(ui->header_baudrate_combo_box, QOverload<int>::of(&QComboBox::activated), iv.pcon, &PortConnection::BaudrateComboBoxIndexChanged);
    iv.pcon->FindBaudrates();

    connect(iv.pcon, SIGNAL(LostConnection()), this, SLOT(ClearTabs()));
    tab_populator = new TabPopulator(ui, &tab_map_);
    connect(iv.pcon, SIGNAL(TypeStyleFound(int,int,int)), tab_populator, SLOT(PopulateTabs(int,int,int)));

    connect(iv.pcon, SIGNAL(FindSavedValues()), this, SLOT(ShowMotorSavedValues()));

    Defaults *def = new Defaults(ui->default_box, "/Resources/Defaults/");

    connect(ui->default_box, QOverload<int>::of(&QComboBox::activated), def, &Defaults::DefaultComboBoxIndexChanged);
    connect(ui->default_pushbutton, SIGNAL(clicked()), def, SLOT(LoadDefaultValues()));
    connect(def, SIGNAL(SaveDefaultValues(Json::Value)), this, SLOT(SetDefaults(Json::Value)));

    Firmware *firm = new Firmware();

    connect(ui->boot_mode_button, SIGNAL(clicked()), firm, SLOT(BootModeSelected()));

  }
  catch(const QString &e)
  {
    ui->header_error_label->setText(e);
    return;
  }
}


MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_pushButton_home_clicked()
{
  ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_pushButton_general_clicked()
{
  ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_tuning_clicked()
{
  ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_pushButton_testing_clicked()
{
  ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::on_pushButton_advanced_clicked()
{
  QMessageBox msgBox;
  msgBox.setWindowTitle("WARNING!");
  msgBox.setText("Changing settings in the Advanced tab could compromise the safety features IQ has put in place. Please use extreme caution.\n\nI understand that changing settings in this tab could result in the damage or destruction of my motor.");
  msgBox.setStandardButtons(QMessageBox::Yes);
  msgBox.addButton(QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  if(msgBox.exec() == QMessageBox::Yes)
  {
    ui->stackedWidget->setCurrentIndex(5);
  }
}

void MainWindow::on_pushButton_firmware_clicked()
{
  ui->stackedWidget->setCurrentIndex(4);
}

void MainWindow::ShowMotorSavedValues()
{
  if(iv.pcon->GetConnectionState() == 1)
  {
    for( std::pair<std::string, std::shared_ptr<Tab>> tab: tab_map_)
    {
      tab.second->CheckSavedValues();
    }
  }
  else
  {
    QString error_message = "NO MOTOR CONNECTED, PLEASE CONNECT MOTOR";
    iv.label_message->setText(error_message);
  }
}

void MainWindow::SetDefaults(Json::Value defaults)
{

  if(iv.pcon->GetConnectionState() == 1 && !tab_map_.empty())
  {
    std::map<std::string,double> default_value_map;
    for(uint8_t ii = 0; ii < defaults.size(); ++ii)
    {
      if(!defaults[ii]["Entries"].empty())
      {
        default_value_map.clear();
        std::string tab_descriptor = defaults[ii]["descriptor"].asString() + ".json";
        Json::Value defaults_values = defaults[ii]["Entries"];
        for(uint8_t jj = 0;  jj < defaults_values.size(); ++jj)
        {
          std::string value_descriptor = defaults_values[jj]["descriptor"].asString();
          double value = defaults_values[jj]["value"].asDouble();
          default_value_map[value_descriptor] = value;
        }
        std::map<std::string, std::shared_ptr<Tab>>::const_iterator it;
        it = tab_map_.find(tab_descriptor);
        if(it != tab_map_.end())
        {
          tab_map_[tab_descriptor]->SaveDefaults(default_value_map);
          tab_map_[tab_descriptor]->CheckSavedValues();
        }
        else
        {
          QString error_message = "WRONG DEFAULT SETTINGS SELECTED";
          iv.label_message->setText(error_message);
          return;
        }
      }
    }
    QString success_message = "DEFAULT SETTINGS VALUE SAVED";
    iv.label_message->setText(success_message);
    return;
  }
  else
  {
    QString error_message = "NO MOTOR CONNECTED, PLEASE CONNECT MOTOR";
    iv.label_message->setText(error_message);
    return;
  }
}

void MainWindow::ClearTabs()
{
  tab_map_.clear();
}





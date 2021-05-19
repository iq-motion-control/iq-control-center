#ifndef UPDATER_HPP
#define UPDATER_HPP

#include <QMainWindow>
#include <QObject>
#include <QDebug>
#include "ui_mainwindow.h"
//#include <QtAutoUpdaterGui/UpdateController>
//#include <QtAutoUpdaterGui/UpdateButton>

class Updater: public QObject
{
   Q_OBJECT
  public:
    Updater(Ui::MainWindow *user_in,  QWidget *parent = nullptr);

    void ConnectUpdater();

  private:
    Ui::MainWindow *ui_;
    QWidget *parent_;
    QtAutoUpdater::UpdateController *controller;
    QtAutoUpdater::UpdateButton *updateButton;

  private slots:
    void initializeUpdater();
    void checkUpdate();
};

#endif // UPDATER_HPP

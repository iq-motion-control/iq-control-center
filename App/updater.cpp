#include "updater.hpp"

Updater::Updater(Ui::MainWindow *user_int,  QWidget *parent):
   ui_(user_int),
   parent_(parent),
   controller(nullptr),
   updateButton(new QtAutoUpdater::UpdateButton(parent_))
{
//  updateButton->setVisible(false);
  initializeUpdater();
  controller->start(QtAutoUpdater::UpdateController::InfoLevel);
}

void Updater::ConnectUpdater()
{
  connect(ui_->actionCheck_for_Updates, SIGNAL(triggered()), this, SLOT(checkUpdate()));
}

void Updater::initializeUpdater()
{
  controller = new QtAutoUpdater::UpdateController(QStringLiteral(MAINTENANCETOOL_PATH),qApp);	//Updater app name
  controller->setDetailedUpdateInfo(true);
  updateButton->setController(controller);
}

//Starts update check process
void Updater::checkUpdate()
{
  controller->start(QtAutoUpdater::UpdateController::ProgressLevel);	//Check for updates. Displays a progress bar when searching
}

#include "main.h"
#include "resource_pack.h"

ResourcePack::ResourcePack(){
  // Get current application path to use for importing Resource Pack.
  currentAppPath = QCoreApplication::applicationDirPath();
}

ResourcePack::~ResourcePack(){}

void ResourcePack::importResourcePackFromPath(QString zipFilePath) {
  if(zipFilePath != ""){
    iv.pcon->AddToLog("Selected .zip file: " + zipFilePath);

    QFileInfo fileInfo(zipFilePath);
    // Get the base name of the zip file, which is the name of the file minus '.zip'.
    resourcePackBaseName = fileInfo.baseName();

    // Create a temporary directory to store the extracted resource files.
    QString tempDirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/control_center_resources";
    QDir tempDir(tempDirPath);

    iv.pcon->AddToLog("Temorary directory path:" + tempDirPath);

    if(tempDir.path() != ""){
      JlCompress extractTool;
      QStringList resourcePackFileList = extractTool.getFileList(zipFilePath);

      for(uint8_t i=0; i< resourcePackFileList.length(); i++){
        QString filePath = resourcePackFileList.at(i);
        QString tempResourceFilePath = tempDirPath + "/" + filePath;

        if(filePath.contains(".json")){
          iv.pcon->AddToLog("Temporary resource file path:" + filePath);
          QString extracted_file = extractTool.extractFile(zipFilePath, filePath, tempResourceFilePath);
          iv.pcon->AddToLog("Extracted:" + extracted_file);

          // Set the required file permissions before copying over to main Resources directory
          setFilePermissions(tempResourceFilePath);

          if(filePath.contains(resourcePackBaseName)){
            // Get the name of the resource file without including the name of the .zip folder
            QStringList splitPath = filePath.split(resourcePackBaseName);

            QString resourcesPath = currentAppPath + "/Resources" + splitPath[1];
            iv.pcon->AddToLog("Copying file: " + tempResourceFilePath + " to: " + resourcesPath);

            QFile::copy(tempResourceFilePath, resourcesPath);
          }else{
            iv.pcon->AddToLog("Invalid .json file!");
          }
        }
      }
    }else{
      iv.pcon->AddToLog("Temporary directory not valid!");
    }
  }else {
    iv.pcon->AddToLog("Import Resource Pack clicked but no .zip file selected.");
  }
}

void ResourcePack::displayMessageBox(){
      QMessageBox msgBox;
      msgBox.setWindowTitle("Application restart required");

      QString msg;
      msg = "Resource files imported successfully. Please close and restart the application to properly load the imported resource files.";
      msgBox.setText(msg);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setDefaultButton(QMessageBox::Ok);
      iv.pcon->AddToLog(msg);

      if(msgBox.exec() == QMessageBox::Ok){
          return;
      }
}

void ResourcePack::setFilePermissions(QString filePath){
    QFile * tempFile = new QFile(filePath);
    if(tempFile->setPermissions(filePath, QFileDevice::ReadOther | QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadUser
                                | QFileDevice::WriteOther | QFileDevice::WriteOwner | QFileDevice::WriteGroup | QFileDevice::WriteUser
                                | QFileDevice::ExeOther | QFileDevice::ExeOwner | QFileDevice::ExeGroup | QFileDevice::ExeUser
                                )){;
        iv.pcon->AddToLog("Updated permissions for: " + filePath);
    }else{
        iv.pcon->AddToLog("Failed to update permissions: "+ filePath);
    }
    delete tempFile;
}

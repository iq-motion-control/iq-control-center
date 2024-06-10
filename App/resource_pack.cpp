#include "main.h"
#include "resource_pack.h"

ResourcePack::ResourcePack(){
  // Get current application path to use for importing Resource Pack.
  currentAppPath = QCoreApplication::applicationDirPath();
}

ResourcePack::~ResourcePack(){}

void ResourcePack::importResourcePackFromPath(QString zipFilePath) {
  iv.pcon->AddToLog("zipFilePath: " + zipFilePath);

  QFileInfo fileInfo(zipFilePath);
  // Get the base name of the zip file, which is the name of the file minus '.zip'.
  //resourcePackBaseName = fileInfo.baseName();
  QSysInfo sysInfo;
  QString kernelType = sysInfo.kernelType();
  if(kernelType == "darwin"){
      iv.pcon->AddToLog("kernelType: " + kernelType);
      QString tempDirPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/control_center_resources";
//      QString dumpPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/Dump";
      QDir tempDir(tempDirPath);
      iv.pcon->AddToLog("TempDirPath:" + tempDirPath);
      if(tempDir.path() != ""){
        //resourcePackExtractPath = tempDirPath;
        iv.pcon->AddToLog("Temporary directory path:" + tempDir.path());
        JlCompress extractTool;
        QStringList stringList = extractTool.getFileList(zipFilePath);
//        iv.pcon->AddToLog("String List:"+stringList)
        for(uint8_t i=0; i< stringList.length(); i++){
            QString filePath = stringList.at(i);
            iv.pcon->AddToLog("Temporary resource file path:" + filePath);
            //QString extracted_file = extractTool.extractFile(zipFilePath,filePath,"/Users/iqmotioncontrol/Desktop/Dump/" + filePath);
            QString extracted_file = extractTool.extractFile(zipFilePath,filePath,tempDirPath+"/"+filePath);
            iv.pcon->AddToLog("Extracted:"+extracted_file);
//            setFilePermissions("/Users/iqmotioncontrol/Desktop/Dump/" + filePath);

            setFilePermissions(tempDirPath+"/"+filePath);

            //setFilePermissions(tempDirPath + filePath);

        }

      }
  }else{
      if(zipFilePath != ""){
        iv.pcon->AddToLog("Resource Pack selected: " + zipFilePath);
        JlCompress extractTool;

        // Create a temporary directory to store the extracted files. This object gets automatically deleted once out of scope.
        QTemporaryDir tempDir;
        if(tempDir.isValid()){
          resourcePackExtractPath = tempDir.path();
          iv.pcon->AddToLog("Temporary directory path:" + resourcePackExtractPath);

          // Extract contents of .zip file into temporary directory.
          extractTool.extractDir(zipFilePath, resourcePackExtractPath);
          QDirIterator dirIt(resourcePackExtractPath, QDirIterator::Subdirectories);

          // Iterate through each resource file in the temporary directory and copy to the main Resources directory
          while(dirIt.hasNext()){
            QString filePath = dirIt.next();
            if(filePath.contains(".json")){
                // Get the path of each resource file.
                QStringList splitPath = filePath.split(resourcePackBaseName);

                // Set the appropriate read permissions to make sure they can be accessed.
                setFilePermissions(filePath);

                QString resourcesPath = currentAppPath + "/Resources" + splitPath[1];
                iv.pcon->AddToLog("Copying file: " + filePath + " to: " + resourcesPath);
                QFile::copy(filePath, resourcesPath);
            }
          }
          displayMessageBox();
        }else{
          iv.pcon->AddToLog("Temporary directory not valid!");
        }
      }else{
        iv.pcon->AddToLog("Import Resource Pack clicked but no .zip file selected.");
      }
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
    // Using a QTemporaryFile object because it automatically gets deleted once out of scope.
    QFile * tempFile = new QFile(filePath);
//    if(tempFile->setPermissions(filePath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ReadUser | QFileDevice::WriteUser
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

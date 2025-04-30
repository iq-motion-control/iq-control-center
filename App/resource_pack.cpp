#include "main.h"
#include "resource_pack.h"

ResourcePack::ResourcePack(QString appDataSessionResourcesPath, QString appDataImportedResourcesPath){
  // Set the SessionResourceFiles path in AppData provided by mainwindow
  this->appDataSessionResourcesPath = appDataSessionResourcesPath;
  // Set the ImportedResourceFiles path in AppData provided by mainwindow
  this->appDataImportedResourcesPath = appDataImportedResourcesPath;

  // Create the QDir object for the ImportedResourceFiles directory in AppData
  appDataImportedResourcesDirectory = QDir(this->appDataImportedResourcesPath);
  // Make the ImportedResourceFiles directory in AppData if it doesn't already exist
  if (!appDataImportedResourcesDirectory.exists()){
    iv.pcon->AddToLog("ImportedResourcFiles does not exist in AppData. Creating now: " + this->appDataImportedResourcesPath);
    appDataImportedResourcesDirectory.mkpath(".");
  }
}

ResourcePack::~ResourcePack(){}

void ResourcePack::importResourcePackFromPath(QString zipFilePath) {
  if(!zipFilePath.isEmpty()){
    iv.pcon->AddToLog("Selected .zip file: " + zipFilePath);

    QFileInfo zipFileInfo(zipFilePath);
    // Get the base name of the .zip file, which is the name of the file minus '.zip'.
    QString zipFileBaseName = zipFileInfo.baseName();

    if(appDataImportedResourcesDirectory.exists()){
      JlCompress extractTool;
      // Get the list of paths in the .zip file
      QStringList resourcePackFileList = extractTool.getFileList(zipFilePath);

      iv.pcon->AddToLog("Extracting from: " + zipFilePath + " to: " + appDataImportedResourcesPath);

      // Iterate through the resource pack file list and extracted each file into ImportedResourceFiles
      if(!resourcePackFileList.isEmpty()){
        // Use qAsConst() here to apply const to QStringList because QtCreator complains
        for(const QString& resourceFile : qAsConst(resourcePackFileList)) {
          int zipFileBaseNameLength = zipFileBaseName.length() + 1; // Add 1 to length to include trailing slash
          // Get the path of the resource file to be imported only keeping everything after the zipFileBaseName
          // We don't want to keep the top-level directory as the base name of the .zip file, we only want the subdirectories that contain the resource files
          QString resourceFileName = resourceFile.mid(zipFileBaseNameLength);

          // Construct the destination path where the extracted resource file will be stored in ImportedResourceFiles
          QString destinationImportedResourcesPath = appDataImportedResourcesPath + resourceFileName;

          // Extract resource file from .zip file to ImportedResourceFiles in AppData
          iv.pcon->AddToLog("Extracting Resource file: " + resourceFile + " to: " + destinationImportedResourcesPath);

          // extractFile() will return the path of the extracted resource file at the destination
          // This will extract the resource file and store it in ImportedResourceFiles without the top-level base name directory of the .zip file
          QString extractedFilePath = extractTool.extractFile(zipFilePath, resourceFile, destinationImportedResourcesPath);

          // Create a QFileInfo object to check if the extracted resource file is a file before copying over to SessionResourceFiles
          QFileInfo fileInfo(extractedFilePath);
          if(fileInfo.isFile()){
            QString destinationSessionResourcesPath = appDataSessionResourcesPath + resourceFileName;
            iv.pcon->AddToLog("Copying Resource file: " + extractedFilePath + " to: " + destinationSessionResourcesPath);

            // Remove existing resource file, new resource file will overwrite any existing ones
            QFile::remove(destinationSessionResourcesPath);
            QFile::copy(extractedFilePath, destinationSessionResourcesPath);
          }
        }
        iv.pcon->AddToLog("Extracted Resource Files");
        displayMessageBox("Application restart required", "Resource files imported successfully. Please close and restart the application to properly load the imported resource files.");
      } else{
        iv.pcon->AddToLog("Failed to extract resource pack!");
      }
    }else {
      iv.pcon->AddToLog("Import Resource Pack clicked but no .zip file selected.");
    }
  }
}

void ResourcePack::displayMessageBox(QString title, QString message){
      QMessageBox msgBox;
      msgBox.setWindowTitle(title);

      msgBox.setText(message);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.setDefaultButton(QMessageBox::Ok);
      iv.pcon->AddToLog(message);

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

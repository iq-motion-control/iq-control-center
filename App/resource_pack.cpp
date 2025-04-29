#include "main.h"
#include "resource_pack.h"
#include "QDebug"

ResourcePack::ResourcePack(QString appDataSessionResourcesPath, QString appDataImportedResourcesPath){
  // Set the SessionResourceFiles path in AppData provided by mainwindow
  this->appDataSessionResourcesPath = appDataSessionResourcesPath;
  // Set the ImportedResourceFiles path in AppData provided by mainwindow
  this->appDataImportedResourcesPath = appDataImportedResourcesPath;

  // Create the QDir object for the ImportedResourceFiles directory in AppData
  appDataImportedResourcesDirectory = QDir(this->appDataImportedResourcesPath);
  // Make the ImportedResourceFiles directory in AppData if it doesn't already exist
  if (!appDataImportedResourcesDirectory.exists()){
    qDebug() << "ImportedResourcesFiles does not exist in AppData. Creating now: " << this->appDataImportedResourcesPath;
    iv.pcon->AddToLog("ImportedResourcFiles does not exist in AppData. Creating now: " + this->appDataImportedResourcesPath);
    appDataImportedResourcesDirectory.mkpath(".");
  }
}

ResourcePack::~ResourcePack(){}

void ResourcePack::importResourcePackFromPath(QString zipFilePath) {
  if(!zipFilePath.isEmpty()){
    iv.pcon->AddToLog("Selected .zip file: " + zipFilePath);

    QFileInfo zipFileInfo(zipFilePath);
    // Get the base name of the zip file, which is the name of the file minus '.zip'.
    QString zipFileBaseName = zipFileInfo.baseName();

    if(appDataImportedResourcesDirectory.exists()){
      JlCompress extractTool;
      QStringList resourcePackFileList = extractTool.getFileList(zipFilePath);

      qDebug() << "Extracting from: " << zipFilePath << "to: " << appDataImportedResourcesPath;
      // Extracts contents from zip file to ImportedResourceFiles in AppData
      QStringList extractedResourceFiles = extractTool.extractDir(zipFilePath, appDataImportedResourcesPath);

      // Copy extracted files into SessionResourceFiles
      if(!extractedResourceFiles.isEmpty()){
        // Use qAsConst() here to apply const to QStringList because QtCreator complains
        for(const QString& resourceFile : qAsConst(extractedResourceFiles)) {
          qDebug() << " appDataImportedResourcesPath:" << appDataImportedResourcesPath;
          qDebug() << "zipFileInfo basename: " << zipFileInfo.baseName();
          int appDataImportedResourcesPathLength = appDataImportedResourcesPath.length() + zipFileBaseName.length() + 1; // add 1 to include the trailing slash after the basename
          // Get the path of the imported resource file only keeping everything after the zipFileBaseName
          QString fileName = resourceFile.mid(appDataImportedResourcesPathLength);
          QFileInfo fileInfo(resourceFile);
          if(fileInfo.isFile()){
            qDebug() << "Resource file: " << fileName;
            QString destinationFilePath = appDataSessionResourcesPath + fileName;
            qDebug() << "Copying Resource file: " << resourceFile << " to: " << destinationFilePath;
            // Remove existing resource file, new resource file will overwrite any existing ones
            QFile::remove(destinationFilePath);
            QFile::copy(resourceFile, destinationFilePath);
          }
        }
        qDebug() << "Extracted Resource Files";
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

#include "resource_pack.h"
#include "main.h"

ResourcePack::ResourcePack(){
}

void ResourcePack::importResourcePackFromPath(QString zipFileToImport) {
  resourcePackExtractPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/resource_pack_dir";
  QFileInfo fileInfo(zipFileToImport);
  // Get the base name of the zip file, which is the name of the file minus '.zip'
  resourcePackBaseName = fileInfo.baseName();

  if(zipFileToImport!= ""){
    iv.pcon->AddToLog("Resource Pack selected: " + zipFileToImport);
    iv.pcon->AddToLog("Temp resource pack dir: " + resourcePackExtractPath);

    JlCompress extract_tool;
    //Create a temporary directory to store the extracted files
    QDir tempDir(resourcePackExtractPath);
    resourcePackTempDir.swap(tempDir);
    extract_tool.extractDir(zipFileToImport, resourcePackExtractPath);
    copyToResourceDir();
  }else{
    iv.pcon->AddToLog("Resource Pack path not valid!");
  }
}

void ResourcePack::copyToResourceDir(){
  // Use a QDirIterator object to recursively iterate through each file in the Resource Pack temp directory
  QDirIterator dirIt(resourcePackTempDir, QDirIterator::Subdirectories);
  QString current_path = QCoreApplication::applicationDirPath();

  while(dirIt.hasNext()){
    QString filePath = dirIt.next();

    if(filePath.contains(".json")){
        //If it's a real file go and change all of the possible read permissions to make sure we can get in
        QStringList splitPath = filePath.split(resourcePackBaseName);
        updateResourceFilePermissions(filePath);
        iv.pcon->AddToLog("Updated permissions for: " + splitPath[1]);
        QString resourcesPath = current_path + "/Resources" + splitPath[1];
        iv.pcon->AddToLog("Copying file: " + filePath);
        iv.pcon->AddToLog("To directory: " + resourcesPath);

        QFile::copy(filePath, resourcesPath);
    }
  }
}

void ResourcePack::updateResourceFilePermissions(QString filePath){
  QFile * tempFile = new QFile(filePath);
  tempFile->setPermissions(filePath, QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::ReadUser);
  delete tempFile;
}

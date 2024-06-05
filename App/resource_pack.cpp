#include "resource_pack.h"
#include "main.h"

ResourcePack::ResourcePack(){
  // Get current application path to use for importing Resource Pack.
  currentAppPath = QCoreApplication::applicationDirPath();
}

void ResourcePack::importResourcePackFromPath(QString zipFileToImport) {
  QFileInfo fileInfo(zipFileToImport);
  // Get the base name of the zip file, which is the name of the file minus '.zip'.
  resourcePackBaseName = fileInfo.baseName();

  if(zipFileToImport!= ""){
    iv.pcon->AddToLog("Resource Pack selected: " + zipFileToImport);
    iv.pcon->AddToLog("Temp resource pack dir: " + resourcePackExtractPath);

    JlCompress extractTool;
    // Create a temporary directory to store the extracted files. This object gets automatically deleted once out of scope.
    QTemporaryDir tempDir;
    if(tempDir.isValid()){
      resourcePackExtractPath = tempDir.path();
      iv.pcon->AddToLog("Temporary directory path:" + resourcePackExtractPath);

      // Extract contents of .zip file into temporary directory.
      extractTool.extractDir(zipFileToImport, resourcePackExtractPath);
      QDirIterator dirIt(resourcePackExtractPath, QDirIterator::Subdirectories);

      // Iterate through each resource file in the temporary directory and copy to the main Resources directory
      while(dirIt.hasNext()){
        QString filePath = dirIt.next();
        if(filePath.contains(".json")){
            // Get the path of each resource file.
            QStringList splitPath = filePath.split(resourcePackBaseName);
            //If it's a real file go and change all of the possible read permissions to make sure we can access them.
            updateResourceFilePermissions(filePath);
            iv.pcon->AddToLog("Updated permissions for: " + splitPath[1]);

            QString resourcesPath = currentAppPath + "/Resources" + splitPath[1];
            iv.pcon->AddToLog("Copying file: " + filePath + " to: " + resourcesPath);
            QFile::copy(filePath, resourcesPath);
        }
      }
    }else{
      iv.pcon->AddToLog("Temporary directory not valid!");
    }
  }else{
    iv.pcon->AddToLog("Import Resource Pack clicked but no .zip file selected.");
  }
}

void ResourcePack::updateResourceFilePermissions(QString filePath){
  QFile * tempFile = new QFile(filePath);
  tempFile->setPermissions(filePath, QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::ReadUser);
  delete tempFile;
}

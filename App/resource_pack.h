#ifndef RESOURCE_PACK_H
#define RESOURCE_PACK_H

#include <JlCompress.h>

#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>

/**
 * @brief The ResourcePack class is used to import resource files from a .zip file to the application's Resources directory
 * The .zip file must follow a specific file structure that contains .json files. The zip file must also contain a top level directory with the same name.
 * Here is an example of how to create a Resource Pack called ResourcePack:
 *      1) create a directory called ResourcePack
 *      2) create subdirectories:
 *              Defaults
 *              Firmware
 *              Tabs
 *                  advanced
 *                  general
 *                  testing
 *                  tuning
 *      3) add .json resource files in the appropriate folders
 *      4) Compress the ResourcePack directory and name it ResourcePack.zip
 *      5) This is what the final file structure of the .zip file would look like:
 *          ResourcePack.zip
 *              ResourcePack
 *                  Defaults
 *                      Vertiq_8108_150Kv_speed_KDE30.5x9.7R.json
 *                  Firmware
 *                      48.json
 *                  Tabs
 *                      advanced
 *                          advanced_vertiq_60xx_speed.json
 *                      general
 *                          general_vertiq_60xx_speed.json
 *                      testing
 *                          testing_vertiq_60xx_speed.json
 *                      tuning
 *                          tuning_vertiq_60xx_speed.json
 *
 *      Note: It is really important that the folders are named exactly like above (folder names are case-sensitive)
 */
class ResourcePack : public QObject{
    Q_OBJECT

  public:
    ResourcePack(QString appDataSessionResourcesPath, QString appDataImportedResourcesPath);
   ~ResourcePack();
    /**
     * @brief importResourcePackFromPath Extracts resource files from the .zip file specified by the zipFilePath parameter and copies them to the application's Resources directory
     * @param zipFilePath The path of the .zip file that contains the resource files to be imported
     */
    void importResourcePackFromPath(QString zipFilePath);
    /**
     * @brief displayMessageBox Displays a message box indicating that the resource files were imported successfully. It also tells the user to close and restart the application to properly load the resource files.
     * @param title The text displayed on the title of the message box
     * @param message The test displayed in the body of the message box
     */
    void displayMessageBox(QString title, QString message);

  private:
    /**
     * @brief appDataSessionResourcesPath The SessionResourceFiles path in AppData. This is passed in by mainwindow.
     */
    QString appDataSessionResourcesPath;
    /**
     * @brief appDataImportedResourcesPath The AppData/Resources path in AppData. This is passed in by mainwindow.
     */
    QString appDataImportedResourcesPath;
    /**
     * @brief appDataImportedResourcesDirectory The AppData/Local/ImportedResourceFiles path of the Control Center application. This is used to store imported resource files.
     */
    QDir appDataImportedResourcesDirectory;
    /**
     * @brief setFilePermissions Sets the appropriate Read permissions for the extracted resource files
     * @param filePath The file path of the extracted resource file that needs the appropriate permissions
     */
    void setFilePermissions(QString filePath);

};

#endif // RESOURCE_PACK_H

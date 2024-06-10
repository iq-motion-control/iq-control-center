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
 *                  Advanced
 *                  General
 *                  Testing
 *                  Tuning
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
 *                      Advanced
 *                          advanced_vertiq_60xx_speed.json
 *                      General
 *                          general_vertiq_60xx_speed.json
 *                      Testing
 *                          testing_vertiq_60xx_speed.json
 *                      Tuning
 *                          tuning_vertiq_60xx_speed.json
 *
 */
class ResourcePack : public QObject{
    Q_OBJECT

  public:
    ResourcePack();
   ~ResourcePack();
    /**
     * @brief importResourcePackFromPath Extracts resource files from the .zip file specified by the zipFilePath parameter and copies them to the application's Resources directory
     * @param zipFilePath The path of the .zip file that contains the resource files to be imported
     */
    void importResourcePackFromPath(QString zipFilePath);

  private:
    /**
     * @brief currentAppPath The current path of the Control Center application. This is used to locate the application's Resources directory.
     */
    QString currentAppPath;
    /**
     * @brief resourcePackBaseName The name of the .zip file that contains the resource files to be imported. This name is used when copying files from the temporary directory to the application's Resources directory.
     */
    QString resourcePackBaseName;
    /**
     * @brief displayMessageBox Displays a message box indicating that the resource files were imported successfully. It also tells the user to close and restart the application to properly load the resource files.
     */
    void displayMessageBox();
    /**
     * @brief setFilePermissions Sets the appropriate Read permissions for the extracted resource files
     * @param filePath The file path of the extracted resource file that needs the appropriate permissions
     */
    void setFilePermissions(QString filePath);

};

#endif // RESOURCE_PACK_H

#ifndef RESOURCE_PACK_H
#define RESOURCE_PACK_H

#include <JlCompress.h>

#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>

class ResourcePack : public QObject{
    Q_OBJECT

  public:
    ResourcePack();
    void importResourcePackFromPath(QString zipFilePath);

  private:
    QString currentAppPath;
    QString resourcePackExtractPath;
    QString resourcePackBaseName;
    void displayMessageBox();
    void setFilePermissions(QString filePath);

};

#endif // RESOURCE_PACK_H

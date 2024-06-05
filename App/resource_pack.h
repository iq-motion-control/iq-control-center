#ifndef RESOURCE_PACK_H
#define RESOURCE_PACK_H

#include <QObject>
#include <JlCompress.h>

#include <QDirIterator>

#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTemporaryDir>

class ResourcePack : public QObject{
    Q_OBJECT

  public:
    ResourcePack();
    void importResourcePackFromPath(QString zipFileToImport);

  private:
    QString currentAppPath;
    QString resourcePackExtractPath;
    QString resourcePackBaseName;
    void updateResourceFilePermissions(QString filePath);

};

#endif // RESOURCE_PACK_H

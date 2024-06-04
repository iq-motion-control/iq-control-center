#ifndef RESOURCE_PACK_H
#define RESOURCE_PACK_H

#include <QObject>
#include <JlCompress.h>

#include <QDirIterator>

#include <QFile>
#include <QFileDialog>

#include <QStandardPaths>

class ResourcePack : public QObject{
    Q_OBJECT

  public:
    ResourcePack();
    void importResourcePackFromPath(QString zipFileToImport);

  private slots:

  private:
    QString resourcePackPath;
    QString resourcePackExtractPath = "";
    QString resourcePackBaseName;
    QDir resourcePackTempDir;
    void copyToResourceDir();
    void updateResourceFilePermissions(QString filePath);

};

#endif // RESOURCE_PACK_H

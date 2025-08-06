#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QVariant>

class AppSettings : public QObject {
  Q_OBJECT

 public:
  AppSettings();

  bool load();
  // The QVariant type is used here because the value of the setting can be many different types ex: QString, bool, int, QList
  // There are functions for each type to cast the return values from the get()
  QVariant get(const QString& key, const QVariant& defaultValue = QVariant()) const;
  void set(const QString& key, const QVariant& value);
  bool save() const;

 private:
  // settingsJsonObject gets populated with the values from settings.json in AppData in the load()
  QJsonObject settingsJsonObject;
  QString appDataSettingsPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/Settings/";
  QString settingsFilePath = appDataSettingsPath + "settings.json";

//  bool load();

 signals:
  void settingChanged(const QString& key, const QVariant& value);
};

#endif // APPSETTINGS_H

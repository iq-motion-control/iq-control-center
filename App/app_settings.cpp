#include "app_settings.h"
#include "QDebug"

AppSettings::AppSettings(){
  QDir appSettingsDir(appDataSettingsPath);

  // Create the Settings directory in AppData if it does not exist
  if(!appSettingsDir.exists()){
    qDebug() << "Settings directory does not exist in AppData. Creating now: " << appDataSettingsPath;
    appSettingsDir.mkpath(".");
  }

  load();
}

bool AppSettings::load() {
  // Create a QFile object with the path to the settings.json file in AppData
  QFile settingsFile(settingsFilePath);

  // Check if the settings.json file exists. If not, create the settings.json file.
  if (!settingsFile.exists()) {
    // Create the file with an empty JSON object
    if (settingsFile.open(QIODevice::WriteOnly)) {
      settingsFile.write(QJsonDocument(QJsonObject()).toJson());
      settingsFile.close();
      qDebug() << "Created new settings file at:" << settingsFilePath;
    } else {
      qWarning() << "Failed to create settings file:" << settingsFilePath;
      return false;
    }
  }

  // Open the settings.json file for reading
  if (!settingsFile.open(QIODevice::ReadOnly)) {
    qWarning() << "Could not open settings file for reading:" << settingsFilePath;
    return false;
  }

  QByteArray data = settingsFile.readAll();
  settingsFile.close();

  QJsonParseError parseError;
  QJsonDocument settingsJsonDoc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    qWarning() << "Failed to parse JSON:" << parseError.errorString();
    return false;
  }

  if (!settingsJsonDoc.isObject()) {
    qWarning() << "Settings file does not contain a valid JSON object.";
    return false;
  }

  // Load the settings.json file as a json object
  // Store the settings in settingsJsonObject so that we don't have to frequently open/close the settings.json file for reading
  settingsJsonObject = settingsJsonDoc.object();
  return true;
}


QVariant AppSettings::get(const QString& key, const QVariant& defaultValue) const {
  // Return the settings value if the setting is in settingsJsonObject, otherwise return the provided default value
  if (settingsJsonObject.contains(key)) {
    return settingsJsonObject.value(key).toVariant();
  }
  return defaultValue;
}

void AppSettings::set(const QString& key, const QVariant& value) {
  QJsonValue jsonValue = QJsonValue::fromVariant(value);
  settingsJsonObject.insert(key, jsonValue);
  save();
}

bool AppSettings::save() const {
  QFile settingsFile(settingsFilePath);
  if (settingsFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
    QJsonDocument settingsJsonDoc(settingsJsonObject);
    settingsFile.write(settingsJsonDoc.toJson(QJsonDocument::Indented));
    settingsFile.close();
    return true;
  }
  return false;
}

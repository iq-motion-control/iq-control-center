#include "metadata_handler.hpp"

MetadataHandler::MetadataHandler(PortConnection * pcon):
    pcon_(pcon)
{
}

void MetadataHandler::ExtractMetadata(QString firmware_bin_path_){
    JlCompress extract_tool;
    //Extract to wherever we're running the project right now
    extract_path_ = qApp->applicationDirPath() + "/flash_dir";
    metadata_dir_ = new QDir(extract_path_);
    extract_tool.extractDir(firmware_bin_path_, extract_path_);
}

QString MetadataHandler::GetCombinedBinPath(){
    QString firmware_bin_path = "";

    if(metadata_dir_->exists("combined.bin")){
        firmware_bin_path = extract_path_ + "/combined.bin";
    }else if(metadata_dir_->exists("main.bin")){
        firmware_bin_path = extract_path_ + "/main.bin";
    }

    return firmware_bin_path;
}

QString MetadataHandler::GetBootBinPath(){
    QString firmware_bin_path = "";

    if(metadata_dir_->exists("boot.bin")){
        firmware_bin_path = extract_path_ + "/boot.bin";
    }

    return firmware_bin_path;
}

QString MetadataHandler::GetAppBinPath(){
    QString firmware_bin_path = "";

    if(metadata_dir_->exists("app.bin")){
        firmware_bin_path = extract_path_ + "/app.bin";
    }

    return firmware_bin_path;
}

QString MetadataHandler::GetUpgradeBinPath(){
    QString firmware_bin_path = "";

    if(metadata_dir_->exists("upgrade.bin")){
        firmware_bin_path = extract_path_ + "/upgrade.bin";
    }

    return firmware_bin_path;
}

QString MetadataHandler::GetPathToCorrectBin(QString binTypeRequested){

    if(binTypeRequested == "combined"){
        return GetCombinedBinPath();
    }else if(binTypeRequested == "boot"){
        return GetBootBinPath();
    }else if(binTypeRequested == "app"){
        return GetAppBinPath();
    }else if(binTypeRequested == "upgrade"){
        return GetUpgradeBinPath();
    }

    return "";
}

QJsonArray MetadataHandler::ArrayFromJson(QString pathToJson){
    //Open the file specified by pathToJson
    QFile jsonFile;
    jsonFile.setFileName(pathToJson);

    //Grab all of the data from the json
    jsonFile.open(QIODevice::ReadOnly);
    QString val = jsonFile.readAll();
    jsonFile.close(); //DON'T FORGET TO CLOSE

    //Read the data stored in the json as a json document
    QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
    //Our json stores each entry contained within an array
    //If your json isn't stored as an array this will come back empty
    return jsonDoc.array();
}

QString MetadataHandler::GetMetadataJsonPath(){
    //Go to the path of the extracted directory (saved in this obj)

    //Get a list of all of the files in the folder
    QStringList filesInFolder = metadata_dir_->entryList();

    //Look for the metadata file in the zip directory
    for(int i = 0; i < filesInFolder.size(); i++){
        if(filesInFolder.at(i).contains(".json")){
            return metadata_dir_->filePath(filesInFolder.at(i));
        }
    }

    //Return empty if not there
    return "";
}

QString MetadataHandler::GetErrorType(){
    QString errorType = "";

    //Message if there is an electronics type error
    QString electronicsError = "Firmware is for the wrong Electronics Type. Expected " + errorType.number(pcon_->GetElectronicsType())
                                        + " and got " + errorType.number(to_flash_electronics_type_);
    //Message if there is a hardware error
    QString hardwareError = "Firmware is for the wrong Hardware Type. Expected " + errorType.number(pcon_->GetHardwareType())
            + " and got " + errorType.number(to_flash_hardware_type_);

    //If they're both wrong print everything that's wrong
    //Otherwise just print whats wrong
    if((to_flash_electronics_type_ != pcon_->GetElectronicsType()) && (to_flash_hardware_type_ != pcon_->GetHardwareType())){
        errorType = electronicsError + "\n" + hardwareError;
    }
    else if(to_flash_electronics_type_ != pcon_->GetElectronicsType()){
        errorType = electronicsError;
    }else{
        errorType = hardwareError;
    }
;
    return errorType;
}

void MetadataHandler::ReadMetadata(){
    metadata_array_ = ArrayFromJson(GetMetadataJsonPath());

    //Grabbing data from the first entry (hardare and electronics type)
    QJsonObject safetyObj = metadata_array_.at(0).toObject();
    to_flash_electronics_type_ = safetyObj.value("to_flash_electronics_type").toInt();
    to_flash_hardware_type_ = safetyObj.value("to_flash_hardware_type").toInt();

    //The second entry is an array with the allowed flash types
    QJsonArray allowedFlashingArray = metadata_array_.at(1).toObject().value("allowed_flashing").toArray();
    allowed_flashing_size_ = allowedFlashingArray.size();
    for(int i = 0; i < allowed_flashing_size_; i++){
        QJsonObject obj(allowedFlashingArray.at(i).toObject());
        flash_types_[i] = new FlashType(obj.value("type").toString(), obj.value("start").toString(),
                                        obj.value("length").toInt(), obj.value("major").toInt(),
                                        obj.value("minor").toInt(), obj.value("patch").toInt(),
                                        obj.value("style").toInt());
    }

    //Third entry is firmware style
    firmware_style_ = metadata_array_.at(FIRMWARE_STYLE_INDEX).toObject().value("firmware style").toString();
    //Last entry is version data and release

    FindBinariesInFolder();
}

QStringList MetadataHandler::GetFlashTypes(){
    QStringList retList;

    for(int i = 0; i < allowed_flashing_size_; i++){
        retList.append(flash_types_[i]->GetType());
    }

    return retList;
}

void MetadataHandler::FindBinariesInFolder(){
    QStringList retList;
    QStringList filesInFolder = metadata_dir_->entryList();

    //Find all binary files in the folder
    for(int i = 0; i < filesInFolder.size(); i++){
        if(filesInFolder.at(i).contains(".bin")){
            retList.append(filesInFolder.at(i));
        }
    }

    binaries_in_folder_ = retList;
}

void MetadataHandler::DeleteExtractedFolder(){
    //We are now done with the extracted directory that we made. We should delete it to avoid any issues
    QDir dir(extract_path_);
    dir.removeRecursively();
    extract_path_ = "";
}

bool MetadataHandler::CheckHardwareAndElectronics(){
    return (to_flash_electronics_type_ == pcon_->GetElectronicsType()) && (to_flash_hardware_type_ == pcon_->GetHardwareType());
}

QString MetadataHandler::GetExtractPath(){
    return extract_path_;
}

uint32_t MetadataHandler::GetStartingMemoryFromType(QString type){

    for(int i = 0; i < allowed_flashing_size_; i++){
        if(flash_types_[i]->GetType() == type){
            uint32_t flashStart = flash_types_[i]->GetStart().toUInt(nullptr, 16);
            return flashStart;
        }
    }

    return 0;
}

uint32_t MetadataHandler::GetUpgradeVersion(){
    //We are flashing a new bootloader so tell the motor that it should update its value
    uint32_t upgradeMajor = GetTypesArray(UPGRADE_INDEX)->GetMajor();
    uint32_t upgradeMinor = GetTypesArray(UPGRADE_INDEX)->GetMinor();
    uint32_t upgradePatch = GetTypesArray(UPGRADE_INDEX)->GetPatch();
    uint32_t upgradeStyle = GetTypesArray(UPGRADE_INDEX)->GetPatch();

    return (((upgradeStyle & 0xfff) << 20) | (upgradeMajor & 0x3f) << 14) | ((upgradeMinor & 0x7f) << 7) | ((upgradePatch & 0x7f));
}

uint32_t MetadataHandler::GetBootloaderVersion(){
    //We are flashing a new bootloader so tell the motor that it should update its value
    uint32_t bootMajor = GetTypesArray(BOOT_INDEX)->GetMajor();
    uint32_t bootMinor = GetTypesArray(BOOT_INDEX)->GetMinor();
    uint32_t bootPatch = GetTypesArray(BOOT_INDEX)->GetPatch();
    return ((((bootMajor & 0x3ff) << 22) | (bootMinor & 0x3ff) << 12) | (bootPatch & 0xfff));
}

void MetadataHandler::Reset(Ui::MainWindow * mainWindow){
    DeleteExtractedFolder();
    extract_path_ = "";
    mainWindow->flash_progress_bar->reset();
    mainWindow->recovery_progress->reset();
    pcon_->ClearFirmwareChoices();
}

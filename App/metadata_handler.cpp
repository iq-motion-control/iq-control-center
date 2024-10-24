#include "metadata_handler.hpp"

MetadataHandler::MetadataHandler(){}

MetadataHandler::MetadataHandler(PortConnection * pcon){
    Init(pcon);
}

void MetadataHandler::Init(PortConnection * pcon){
    pcon_ = pcon;
    extract_path_ = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/flash_dir";
}

void MetadataHandler::UpdateAllFilePermissions(){
    QFileInfoList files_in_dir = metadata_dir_.entryInfoList();

    for(uint8_t i = 0; i < files_in_dir.length(); i++){
        QString filePath = files_in_dir.at(i).absoluteFilePath();

        if(filePath.contains(".bin") || filePath.contains(".json")){
            //If it's a real file go and change all of the possible read permissions to make sure we can get in
            QFile * tempFile = new QFile(filePath);
            tempFile->setPermissions(filePath, QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther | QFileDevice::ReadUser);
            delete tempFile;
        }
    }
}

void MetadataHandler::ExtractMetadata(QString firmware_bin_path_){
    JlCompress extract_tool;

    //Create a temporary directory path then give its information with our class variable directory. lets us avoid dynamic allocation
    QDir tempDir(extract_path_);
    metadata_dir_.swap(tempDir);

    extract_tool.extractDir(firmware_bin_path_, extract_path_);
    UpdateAllFilePermissions();
}

QString MetadataHandler::GetCombinedBinPath(){
    QString firmware_bin_path = "";

    if(metadata_dir_.exists("combined.bin")){
        firmware_bin_path = extract_path_ + "/combined.bin";
    }else if(metadata_dir_.exists("main.bin")){
        firmware_bin_path = extract_path_ + "/main.bin";
    }

    return firmware_bin_path;
}

QString MetadataHandler::GetBootBinPath(){
    QString firmware_bin_path = "";

    if(metadata_dir_.exists("boot.bin")){
        firmware_bin_path = extract_path_ + "/boot.bin";
    }

    return firmware_bin_path;
}

QString MetadataHandler::GetAppBinPath(){
    QString firmware_bin_path = "";

    if(metadata_dir_.exists("app.bin")){
        firmware_bin_path = extract_path_ + "/app.bin";
    }

    return firmware_bin_path;
}

QString MetadataHandler::GetUpgradeBinPath(){
    QString firmware_bin_path = "";

    if(metadata_dir_.exists("upgrade.bin")){
        firmware_bin_path = extract_path_ + "/upgrade.bin";
    }

    return firmware_bin_path;
}

QString MetadataHandler::GetPathToCorrectBin(QString binTypeRequested){

    if(binTypeRequested == "combined" || binTypeRequested == "main"){
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
    QStringList filesInFolder = metadata_dir_.entryList();

    //Look for the metadata file in the zip directory
    for(int i = 0; i < filesInFolder.size(); i++){
        if(filesInFolder.at(i).contains(".json")){
            return metadata_dir_.filePath(filesInFolder.at(i));
        }
    }

    //Return empty if not there
    return "";
}

QStringList MetadataHandler::MakeListOfVersionNumbers(QJsonArray types, QJsonArray major_versions){
    QStringList version_numbers ={};
    //Our types and major versions lists are linked, they need to be the same size. Return early and cry about it
    if(types.size() != major_versions.size()){
        pcon_->AddToLog("Type and Version List lengths do not match! Aborting Creating of Version Numbers List!");
        return version_numbers;
    }

    for(int version_pair_index = 0; version_pair_index <types.size(); version_pair_index++){
        int type = types.at(version_pair_index).toInt();
        int major_version = major_versions.at(version_pair_index).toInt();

        QString version_number = QString::number(type)+"."+QString::number(major_version);
        version_numbers.append(version_number);
    }

    return version_numbers;
}

QString MetadataHandler::GetErrorType(int target_hardware_type, int target_hardware_major_version, int target_electronics_type, int target_electronics_major_version){
    QString errorType = "";

    QString wrong_hardware = pcon_->GetHardwareNameFromResources(target_hardware_type, target_hardware_major_version, target_electronics_type, target_electronics_major_version);
    QString correct_hardware = pcon_->GetHardwareNameFromResources(to_flash_hardware_types_, to_flash_hardware_major_versions_, to_flash_electronics_types_, to_flash_electronics_major_versions_);

    QStringList to_flash_hardware_version_numbers = MakeListOfVersionNumbers(to_flash_hardware_types_, to_flash_hardware_major_versions_);
    QStringList to_flash_electronics_version_numbers = MakeListOfVersionNumbers(to_flash_electronics_types_, to_flash_electronics_major_versions_);

    //Message if there is an electronics type error
    QString electronicsError = "Firmware is for the wrong electronics. File expected to be flashing: (" + to_flash_electronics_version_numbers.join(",") + ")"
                                        + ", but the module was reported as: " + QString::number(target_electronics_type) + "." + QString::number(target_electronics_major_version);
    //Message if there is a hardware error
    QString hardwareError = "Firmware is for the wrong hardware. File expected to be flashing: " + correct_hardware + " (" + to_flash_hardware_version_numbers.join(",") + ")"
             + ", but the module was reported as: " + wrong_hardware + " (" + QString::number(target_hardware_type)
            +"."+ QString::number(target_hardware_major_version) + ")";

    //If they're both wrong print everything that's wrong
    //Otherwise just print whats wrong

//    bool electronics_error = (to_flash_electronics_type_!= target_electronics_type) || (to_flash_electronics_major_version_ != target_electronics_major_version);
//    bool hardware_error = (to_flash_hardware_type_!= target_hardware_type) || (to_flash_hardware_major_version_ != target_hardware_major_version);
    //The one that doesn't have the target in its to flash list is the one with the error. So we can just check if its in the lists and invert it.
    bool hardware_error = !(CheckIfTargetVersionInVersionLists(to_flash_hardware_types_, to_flash_hardware_major_versions_, target_hardware_type, target_hardware_major_version));
    bool electronics_error = !(CheckIfTargetVersionInVersionLists(to_flash_electronics_types_, to_flash_electronics_major_versions_, target_electronics_type, target_electronics_major_version));

    if(electronics_error && hardware_error ){
        errorType = electronicsError + "\n" + hardwareError;
    }
    else if(electronics_error){
        errorType = electronicsError;
    }else{
        errorType = hardwareError;
    }
;
    return errorType;
}

//FRED NOTE: PROBABLY A MORE SPECIFIC NAME
QJsonArray MetadataHandler::GetListFromJSONObjectEntry(QJsonObject object, QString key, int undefined_value){
    QJsonValue raw = object.value(key);

    //It may be an integer already, or it may be an array already. If its an array, great.
    if(raw.isArray()){
        return raw.toArray();
    }else{
        //It's undefined, lets set it to our undefined value first as an integer, to keep our old backwards compatibility
        if(raw.isUndefined()){
            raw = QJsonValue(undefined_value);
        }

        //Ok it either came in as an integer, or we made it one after it was undefined. Now we can shove it into an array
        QJsonArray array;
        array.append(raw);
        return array;
    }
}

void MetadataHandler::ReadMetadata(){
    metadata_array_ = ArrayFromJson(GetMetadataJsonPath());

    //Grabbing data from the first entry (hardware and electronics type)
    QJsonObject safetyObj = metadata_array_.at(0).toObject();

    //Get these, they may be integers or lists. We need to still handle the old single integer style.
    //But those will hopefully just get spat out as arrays by toArray?
    //We need to handle it appropriately, since both single integers and lists can be a thing.
    to_flash_electronics_types_ = GetListFromJSONObjectEntry(safetyObj, "to_flash_electronics_type");
    to_flash_hardware_types_ = GetListFromJSONObjectEntry(safetyObj, "to_flash_hardware_type");

    QJsonDocument doc;
    doc.setArray(to_flash_electronics_types_);
    pcon_->AddToLog("Electronic Types: "+ QString(doc.toJson()));

    doc.setArray(to_flash_hardware_types_);
    pcon_->AddToLog("Hardware Types: "+ QString(doc.toJson()));


    //Older zip file metadata files may not contain this information on major versions. If the key is not present,
    //the value function call will return an undefined value. toInt will return its default value when
    //called on an undefined value, and I have explicitly set the default value to be 0 here to show
    //that we want to assume a major version of 0 if there is no major version specified in the zip file
    //metdata
//    to_flash_electronics_major_version_ = safetyObj.value("to_flash_electronics_major").toInt(0);
//    to_flash_hardware_major_version_ = safetyObj.value("to_flash_hardware_major").toInt(0);

    to_flash_electronics_major_versions_ = GetListFromJSONObjectEntry(safetyObj, "to_flash_electronics_major");
    to_flash_hardware_major_versions_ = GetListFromJSONObjectEntry(safetyObj, "to_flash_hardware_major");

    doc.setArray(to_flash_electronics_major_versions_);
    pcon_->AddToLog("Electronic Majors: "+ QString(doc.toJson()));

    doc.setArray(to_flash_hardware_major_versions_);
    pcon_->AddToLog("Hardware Majors: "+ QString(doc.toJson()));

    //FRED TODO: Should I do a check to confirm that the type and major version lists are the same length?
    //Or handle that later?

    //The second entry is an array with the allowed flash types
    QJsonArray allowedFlashingArray = metadata_array_.at(1).toObject().value("allowed_flashing").toArray();
    allowed_flashing_size_ = allowedFlashingArray.size();
    for(int i = 0; i < allowed_flashing_size_; i++){
        QJsonObject obj(allowedFlashingArray.at(i).toObject());
        flash_types_[i].Init(obj.value("type").toString(), obj.value("start").toString(),
                                        obj.value("length").toInt(), obj.value("major").toInt(),
                                        obj.value("minor").toInt(), obj.value("patch").toInt(),
                                        obj.value("style").toInt());
    }

    //Third entry is firmware style
    firmware_style_ = metadata_array_.at(FIRMWARE_STYLE_INDEX).toObject().value("firmware style").toString();
    //Last entry is version data and release

    FindBinariesInFolder();
}

//void MetadataHandler::ReadMetadata(){
//    metadata_array_ = ArrayFromJson(GetMetadataJsonPath());

//    //Grabbing data from the first entry (hardware and electronics type)
//    QJsonObject safetyObj = metadata_array_.at(0).toObject();
//    to_flash_electronics_type_ = safetyObj.value("to_flash_electronics_type").toInt();
//    to_flash_hardware_type_ = safetyObj.value("to_flash_hardware_type").toInt();

//    //Older zip file metadata files may not contain this information on major versions. If the key is not present,
//    //the value function call will return an undefined value. toInt will return its default value when
//    //called on an undefined value, and I have explicitly set the default value to be 0 here to show
//    //that we want to assume a major version of 0 if there is no major version specified in the zip file
//    //metdata
//    to_flash_electronics_major_version_ = safetyObj.value("to_flash_electronics_major").toInt(0);
//    to_flash_hardware_major_version_ = safetyObj.value("to_flash_hardware_major").toInt(0);

//    //The second entry is an array with the allowed flash types
//    QJsonArray allowedFlashingArray = metadata_array_.at(1).toObject().value("allowed_flashing").toArray();
//    allowed_flashing_size_ = allowedFlashingArray.size();
//    for(int i = 0; i < allowed_flashing_size_; i++){
//        QJsonObject obj(allowedFlashingArray.at(i).toObject());
//        flash_types_[i].Init(obj.value("type").toString(), obj.value("start").toString(),
//                                        obj.value("length").toInt(), obj.value("major").toInt(),
//                                        obj.value("minor").toInt(), obj.value("patch").toInt(),
//                                        obj.value("style").toInt());
//    }

//    //Third entry is firmware style
//    firmware_style_ = metadata_array_.at(FIRMWARE_STYLE_INDEX).toObject().value("firmware style").toString();
//    //Last entry is version data and release

//    FindBinariesInFolder();
//}

QStringList MetadataHandler::GetFlashTypes(){
    QStringList retList;

    for(int i = 0; i < allowed_flashing_size_; i++){
        retList.append(flash_types_[i].GetType());
    }

    return retList;
}

void MetadataHandler::FindBinariesInFolder(){
    QStringList retList;
    QStringList filesInFolder = metadata_dir_.entryList();

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
    //If extract path is "", it can delete a lot of files unexpectedly! It should be impossible to wind up
    //in that case, but checking to be extra careful
    if(extract_path_ != ""){
        QDir dir(extract_path_);
        dir.removeRecursively();
    }
}

bool MetadataHandler::CheckHardwareAndElectronics(int target_hardware_type, int target_hardware_major_version, int target_electronics_type, int target_electronics_major_version){

    //FRED NOTE: Need to fix the get name stuff eventually, but not yet
    QString target_module_name = pcon_->GetHardwareNameFromResources(to_flash_hardware_types_, to_flash_hardware_major_versions_, to_flash_electronics_types_, to_flash_electronics_major_versions_);

    //If the targets come in as -1, then we know we guessed wrong. Flash a warning
    //"Hey you're about to flash firmware meant for [get name for the module]. you sure about this?"
    //If they're not sure about this return false.
    //If we guessed right, actually compare the values

    //This is intended to check if we guessed wrong, so checking only the types is sufficient, no need to check the major versions as well.
    if(target_hardware_type == -1 && target_electronics_type == -1){
        QMessageBox msgBox;
        msgBox.addButton("Yes", QMessageBox::YesRole);
        QAbstractButton * wrongButton = msgBox.addButton("No", QMessageBox::NoRole);

        msgBox.setWindowTitle("Is Firmware Correct?");

        QString text = "You are about to flash firmware meant for the module(s): " + target_module_name + ". Before selecting Recover "
                       "please ensure this is firmware is correct for your module. Is this firmware correct?";

        msgBox.setText(text);
        msgBox.exec();

        if(msgBox.clickedButton() == wrongButton){
            return false;
        }

        pcon_->AddToLog("Flashing firmware meant for: " + target_module_name);
        return true;
    }


    //FRED NOTE: Now this is where we actually do the check normally. Before it was easy, just compare the numbers. Now we need to be a bit smarter. Need to iterate through
    //and see if we can find our match

    bool hardware_match = CheckIfTargetVersionInVersionLists(to_flash_hardware_types_, to_flash_hardware_major_versions_, target_hardware_type, target_hardware_major_version);
    bool electronics_match = CheckIfTargetVersionInVersionLists(to_flash_electronics_types_, to_flash_electronics_major_versions_, target_electronics_type, target_electronics_major_version);
    pcon_->AddToLog("Hardware Match: "+QString(hardware_match));
    pcon_->AddToLog("Electronics Match: "+QString(electronics_match));

    return (hardware_match && electronics_match);
}

//Well, I guess we need to:
//-grab the paired type and major version
//-See if they match our target
//-If not, grab the next pair and repeat
//-If we ever match, tell us
bool MetadataHandler::CheckIfTargetVersionInVersionLists(QJsonArray types, QJsonArray major_versions, int target_type, int target_major_version){
    //Our types and major versions lists are linked, they need to be the same size
    if(types.size() != major_versions.size()){
        pcon_->AddToLog("Type and Version List lengths do not match! Aborting flash!");
        return false;
    }

    for(int version_pair_index = 0; version_pair_index <types.size(); version_pair_index++){
        int to_flash_type = types.at(version_pair_index).toInt();
        int to_flash_major_version = major_versions.at(version_pair_index).toInt();

        if(((to_flash_type == target_type) && (to_flash_major_version == target_major_version))){
            //Perfect match!
            return true;
        }
    }

    //We never matched
    return false;
}


QString MetadataHandler::GetExtractPath(){
    return extract_path_;
}

uint32_t MetadataHandler::GetStartingMemoryFromType(QString type){

    for(int i = 0; i < allowed_flashing_size_; i++){
        if(flash_types_[i].GetType() == type){
            uint32_t flashStart = flash_types_[i].GetStart().toUInt(nullptr, 16);
            return flashStart;
        }
    }

    return 0;
}

uint32_t MetadataHandler::GetUpgradeVersion(){
    //We are flashing a new bootloader so tell the motor that it should update its value
    uint32_t upgradeMajor = GetTypesArray(UPGRADE_INDEX).GetMajor();
    uint32_t upgradeMinor = GetTypesArray(UPGRADE_INDEX).GetMinor();
    uint32_t upgradePatch = GetTypesArray(UPGRADE_INDEX).GetPatch();
    uint32_t upgradeStyle = GetTypesArray(UPGRADE_INDEX).GetStyle();

    return (((upgradeStyle & 0xfff) << 20) | (upgradeMajor & 0x3f) << 14) | ((upgradeMinor & 0x7f) << 7) | ((upgradePatch & 0x7f));
}

uint32_t MetadataHandler::GetBootloaderVersion(){
    //We are flashing a new bootloader so tell the motor that it should update its value
    uint32_t bootStyle = GetTypesArray(BOOT_INDEX).GetStyle();
    uint32_t bootMajor = GetTypesArray(BOOT_INDEX).GetMajor();
    uint32_t bootMinor = GetTypesArray(BOOT_INDEX).GetMinor();
    uint32_t bootPatch = GetTypesArray(BOOT_INDEX).GetPatch();
    return (((bootStyle & 0xfff) << 20) | ((bootMajor & 0x7f) << 14) |((bootMinor & 0x7f) << 7) | bootPatch);
}

void MetadataHandler::Reset(Ui::MainWindow * mainWindow){
    DeleteExtractedFolder();
    mainWindow->flash_progress_bar->reset();
    mainWindow->recovery_progress->reset();
    pcon_->ClearFirmwareChoices();
}

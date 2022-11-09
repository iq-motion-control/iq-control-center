#include "metadata_handler.hpp"

MetadataHandler::MetadataHandler(PortConnection * pcon):
    pcon_(pcon)
{
}

void MetadataHandler::ExtractMetadata(QString firmware_bin_path_){
    JlCompress extract_tool;
    //Extract to wherever we're running the project right now
    extract_path_ = qApp->applicationDirPath() + "/flash_dir";
    extract_tool.extractDir(firmware_bin_path_, extract_path_);
}

QString MetadataHandler::GetBinPath(){
    QString firmware_bin_path = "";
    QDir firmwareDir(extract_path_);

    if(firmwareDir.exists("combined.bin")){
        firmware_bin_path = extract_path_ + "/combined.bin";
    }else if(firmwareDir.exists("main.bin")){
        firmware_bin_path = extract_path_ + "/main.bin";
    }

    return firmware_bin_path;
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
    QDir curDir(extract_path_);

    //Get a list of all of the files in the folder
    QStringList filesInFolder = curDir.entryList();

    //Look for the metadata file in the zip directory
    for(int i = 0; i < filesInFolder.size(); i++){
        if(filesInFolder.at(i).contains(".json")){
            return curDir.filePath(filesInFolder.at(i));
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

    //EXTEND THIS TO GRAB ALL OF THE IMPORTANT THINGS
}

void MetadataHandler::DeleteExtractedFolder(){
    //We are now done with the extracted directory that we made. We should delete it to avoid any issues
    QDir dir(qApp->applicationDirPath() + "/flash_dir");
    dir.removeRecursively();
    extract_path_ = "";
}

bool MetadataHandler::CheckHardwareAndElectronics(){
    return (to_flash_electronics_type_ == pcon_->GetElectronicsType()) && (to_flash_hardware_type_ == pcon_->GetHardwareType());
}

QString MetadataHandler::GetExtractPath(){
    return extract_path_;
}

void MetadataHandler::Reset(){
    DeleteExtractedFolder();
    extract_path_ = "";
}

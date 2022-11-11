#ifndef METADATAHANDLER_HPP
#define METADATAHANDLER_HPP

#include <QString>
#include <JlCompress.h>

#include <QFile>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

#include "port_connection.h"
#include "flash_type.hpp"

#define MAX_FLASH_TYPES 4

class MetadataHandler
{
public:
    /**
     * @brief MetadataHandler Default constructor
     * @param pcon Give access to shared Port Connection object
     */
    MetadataHandler(PortConnection * pcon);

    /**
     * @brief ExtractMetadata Extract the files out of the compressed metadata package
     * @param pathToMetadata Path to the compressed package
     */
    void ExtractMetadata(QString pathToMetadata);

    /**
     * @brief GetPathToCorrectBin Returns the path to the bin associated with the type of flash requested
     * @param binTypeRequested String describing the type of flash requested
     * @return The path
     */
    QString GetPathToCorrectBin(QString binTypeRequested);

    /**
     * @brief ArrayFromJson Creates a array of json information from a json document
     * @param pathToJson The path to the json
     * @return The created json array
     */
    QJsonArray ArrayFromJson(QString pathToJson);

    /**
     * @brief GetMetadataJson Grabs the data from the metadata file
     * @return A string with the json information
     */
    QString GetMetadataJsonPath();

    /**
     * @brief CheckHardwareAndElectronics Checks the metadata hardware and electronics against the connected motor's
     * @return True if match
     */
    bool CheckHardwareAndElectronics();

    /**
     * @brief GetErrorType Determines if there is a mismatch between the electronics and/or the hardware
     * @param toFlashElectronicsType
     * @param toFlashHardwareType
     * @return
     */
    QString GetErrorType();

    /**
     * @brief GetExtractPath Returns the path to the extracted data
     * @return The path
     */
    QString GetExtractPath();

    /**
     * @brief DeleteExtractedFolder Deletes the extracted folder and all of its data
     */
    void DeleteExtractedFolder();

    /**
     * @brief Reset Reset all values to 0 and delete the extracted file if it exists
     */
    void Reset(Ui::MainWindow * mainWindow);

    /**
     * @brief ReadMetadata Places all of the information into the metadata_array_
     */
    void ReadMetadata();

    /**
     * @brief GetFlashTypes Returns a list of strings of flashes that are available
     * @return the list
     */
    QStringList GetFlashTypes();

    /**
     * @brief BinariesInFolder Returns the names of the binary files contained in the folder
     * @return
     */
    void FindBinariesInFolder();

    /**
     * @brief GetStartingMemoryFromType Given a type of binary you want to flash, return the starting memory location of that section
     * @param type The type of binary you want to use
     * @return The starting memory location
     */
    uint32_t GetStartingMemoryFromType(QString type);

    FlashType* GetTypesArray(int i){ return flash_types_[i]; }

    QStringList GetBinariesInFolder(){ return binaries_in_folder_; }

private:
    QString extract_path_ = "";
    PortConnection * pcon_;

    QJsonArray metadata_array_;

    int to_flash_electronics_type_;
    int to_flash_hardware_type_;

    //Var to hold the actual number of entries in the "allowed flashing" json entry
    int allowed_flashing_size_;
    FlashType* flash_types_[MAX_FLASH_TYPES];

    QString firmware_style_;

    QDir *metadata_dir_;

    QStringList binaries_in_folder_;

    /**
     * @brief GetBinPath Returns the path to a binary file in the extracted folder
     * @return The path
     */
    QString GetCombinedBinPath();

    /**
     * @brief GetBootBinPath Get the path to the binary file in the metadata
     * @return The path
     */
    QString GetBootBinPath();

    /**
     * @brief GetAppBinPath Returns the path to the App binary in the metadata
     * @return The path
     */
    QString GetAppBinPath();

    /**
     * @brief GetUpgradeBinPath Returns the path to the Upgrade binary in the metadata
     * @return The path
     */
    QString GetUpgradeBinPath();

};

#endif // METADATAHANDLER_HPP

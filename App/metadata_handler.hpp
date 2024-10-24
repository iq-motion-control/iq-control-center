#ifndef METADATAHANDLER_HPP
#define METADATAHANDLER_HPP

#include <QString>
#include <JlCompress.h>

#include <QFile>

#include <QStandardPaths>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

#include "port_connection.h"
#include "flash_type.hpp"

#define MAX_FLASH_TYPES 4

//Defintions for the indices of the different entries in the json
#define HARDWARE_ELECTRONICS_INDEX 0
#define FIRMWARE_STYLE_INDEX 2

//Definitions for the indices of the firmware flashing information
#define COMBINED_INDEX 0
#define BOOT_INDEX 1
#define UPGRADE_INDEX 2

class MetadataHandler
{
public:

    /**
     * @brief MetadataHandler default constructor. Lets you pass along PortConnection later
     */
    MetadataHandler();

    /**
     * @brief MetadataHandler Default constructor
     * @param pcon Give access to shared Port Connection object
     */
    MetadataHandler(PortConnection * pcon);

    void Init(PortConnection * pcon);

    /**
     * @brief UpdateAllFilePermissions make sure all files are readable by all users
     */
    void UpdateAllFilePermissions();

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
     * @brief CheckHardwareAndElectronics Checks the metadata hardware and electronics against the connected motor's or previously connected motor
     * @param target_hardware_type The type of the connected hardware that you attempted to flash
     * @param target_hardware_major_version The major version of the connected hardware that you attempted to flash
     * @param target_electronics_type The type of the connected electronics that you attempted to flash
     * @param target_electronics_major_verstion The major version of the connected electronics that you attempted to flash
     * @return True if the connected hardware and electronics match the hardware and electronics from the metadata of the file you are attempting to flash
     */
    bool CheckHardwareAndElectronics(int target_hardware_type, int target_hardware_major_version, int target_electronics_type, int target_electronics_major_version);

    /**
     * @brief GetErrorType Determines what the source of the mismatched metatdata error is, and returns a string detailing the problem.
     * @param target_hardware_type The type of the connected hardware that you attempted to flash
     * @param target_hardware_major_version The major version of the connected hardware that you attempted to flash
     * @param target_electronics_type The type of the connected electronics that you attempted to flash
     * @param target_electronics_major_verstion The major version of the connected electronics that you attempted to flash
     * @return A QString providing details on the nature of the error.
     */
    QString GetErrorType(int target_hardware_type, int target_hardware_major_version, int target_electronics_type, int target_electronics_major_version);

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

    /**
     * @brief GetTypesArray Returns a pointer to a FlashType object from the instances stored in the metadata
     * @param i the index of the value you want
     * @return A pointer to a FlashType object
     */
    FlashType GetTypesArray(int i){ return flash_types_[i]; }

    /**
     * @brief GetBinariesInFolder Returns a list of the bin files present in the zip
     * @return The list
     */
    QStringList GetBinariesInFolder(){ return binaries_in_folder_; }

    /**
     * @brief GetUpgradeVersion Returns the version of the upgrader in the zip
     * @return the raw version value
     */
    uint32_t GetUpgradeVersion();

    /**
     * @brief GetBootloaderVersion Returns the raw value of the version of the bootloader in the zip
     * @return the raw version value
     */
    uint32_t GetBootloaderVersion();

private:

    ////////////////////////////////
    /// Private Variables

    /**
     * @brief extract_path_ The path to the extracted folder from the zip input
     */
    QString extract_path_ = "";

    /**
     * @brief pcon_ A pointer to the port connection object used throughout the project
     */
    PortConnection * pcon_;

    /**
     * @brief metadata_array_ stores the information from the metadata json
     */
    QJsonArray metadata_array_;

    /**
//     * @brief to_flash_electronics_type_ The type of electronics that this metadata expects to flash
//     */
//    int to_flash_electronics_type_;

//    /**
//     * @brief to_flash_electronics_type_ The major version of electronics that this metadata expects to flash
//     */
//    int to_flash_electronics_major_version_;

//    /**
//     * @brief to_flash_hardware_type_ The type of hardware that this metadata expects to flash
//     */
//    int to_flash_hardware_type_;

//    /**
//     * @brief to_flash_hardware_type_ The major version of hardware that this metadata expects to flash
//     */
//    int to_flash_hardware_major_version_;

    /**
     * @brief to_flash_electronics_type_ The type of electronics that this metadata expects to flash
     */
    QJsonArray to_flash_electronics_types_;

    /**
     * @brief to_flash_electronics_type_ The major version of electronics that this metadata expects to flash
     */
    QJsonArray to_flash_electronics_major_versions_;

    /**
     * @brief to_flash_hardware_type_ The type of hardware that this metadata expects to flash
     */
    QJsonArray to_flash_hardware_types_;

    /**
     * @brief to_flash_hardware_type_ The major version of hardware that this metadata expects to flash
     */
    QJsonArray to_flash_hardware_major_versions_;

    /**
     * @brief allowed_flashing_size_ The number of different styles of flash that can be performed based on this archive
     */
    int allowed_flashing_size_;

    /**
     * @brief flash_types_ An array that stores all of the information for how to flash each section
     */
    FlashType flash_types_[MAX_FLASH_TYPES];

    /**
     * @brief firmware_style_ Holds the style of firmware in this archive (speed, servo, etc.)
     */
    QString firmware_style_;

    /**
     * @brief metadata_dir_ A pointer to the directory holding the metadata
     */
    QDir metadata_dir_;

    /**
     * @brief binaries_in_folder_ A list of the binaries stored in the archive
     */
    QStringList binaries_in_folder_;

    /////////////////////
    /// Private functions

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

    QJsonArray GetListFromJSONObjectEntry(QJsonObject object, QString key, int undefined_value=0);

    bool CheckIfTargetVersionInVersionLists(QJsonArray types, QJsonArray major_versions, int target_type, int target_major_version);

    //Make a list of the possible version numbers from these types and major version, e.g. [34.1, 34.2, 35.2]
    QStringList MakeListOfVersionNumbers(QJsonArray types, QJsonArray major_versions);

};

#endif // METADATAHANDLER_HPP

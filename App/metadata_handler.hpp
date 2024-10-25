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
     * @brief to_flash_electronics_types_ An array of the types of electronics that this metadata expects to flash. This
     * is paired with the major versions to make an actual full version. Should be the same length as matching major
     * versions array. The type at a given index in this list must be combined with the major version at that same index
     * in the major versions list to create the full version meant to be indicated.
     */
    QJsonArray to_flash_electronics_types_;

    /**
     * @brief to_flash_electronics_major_versions_ An array of the major versions of electronics that this metadata expects to flash. This
     * is paired with the types to make an actual full version. Should be the same length as matching
     * types array. The type at a given index in this list must be combined with the type at that same index
     * in the types list to create the full version meant to be indicated.
     */
    QJsonArray to_flash_electronics_major_versions_;

    /**
     * @brief to_flash_hardware_types_ An array of the types of hardware that this metadata expects to flash. This
     * is paired with the major versions to make an actual full version. Should be the same length as matching major
     * versions array. The type at a given index in this list must be combined with the major version at that same index
     * in the major versions list to create the full version meant to be indicated.
     */
    QJsonArray to_flash_hardware_types_;

    /**
     * @brief to_flash_hardware_major_versions_ An array of the major versions of hardwares that this metadata expects to flash. This
     * is paired with the types to make an actual full version. Should be the same length as matching
     * types array. The type at a given index in this list must be combined with the type at that same index
     * in the types list to create the full version meant to be indicated.
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

    /**
     * @brief GetMetadataVersionEntryAsArray Takes an entry from a metadta file that may contain an integer or an array of types and major versions
     * and converts it into a consistent style of array to work in either case. Useful for handling metadta with either integers or arrays of versions the same.
     * @param object A QJsonObject that holds the type or version entries
     * @param key The key string to use to extract the specific entry you want to convert
     * @return The entry of types or versions as an array
     */
    QJsonArray GetMetadataVersionEntryAsArray(QJsonObject object, QString key);

    /**
     * @brief CheckIfTargetVersionInVersionLists Checks if our paired type and version lists have a match in them for our target type and major version.
     * For example, if our lists were [34, 35] and [1,2], that would correspond to versions of 34.1 and 35.2. If the target is 34.1 or 35.2, we will
     * have a match. Otherwise, there will be no match.
     * @param types List of types to check for matches, paired with major_versions
     * @param major_versions List of major versions to check for matches, paired with types
     * @param target_type The target type to try and match from our lists of possibilities
     * @param target_major_version The target major version to try and match from our lists of possibilities
     * @return True if we match the target version, false if we don't
     */
    bool CheckIfTargetVersionInVersionLists(QJsonArray types, QJsonArray major_versions, int target_type, int target_major_version);

    /**
     * @brief Make a string list of our possible full version strings, useful for displaying all the versions that would have been acceptable.
     * For example, [34, 35] and [1,2] would produce a list of [34.1, 35.2]
     * @param types List of types to use to build version strings. Paired with major_versions
     * @param major_versions List of major versions to use to build version strings. Paired with types
     * @return A list of all the possible version strings given our types and major versions
     */
    QStringList MakeListOfVersionNumbers(QJsonArray types, QJsonArray major_versions);

};

#endif // METADATAHANDLER_HPP

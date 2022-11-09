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
     * @brief GetBinPath Returns the path to a binary file in the extracted folder
     * @return The path
     */
    QString GetBinPath();

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
    void Reset();

    /**
     * @brief ReadMetadata Places all of the information into the metadata_array_
     */
    void ReadMetadata();

private:
    QString extract_path_ = "";
    PortConnection * pcon_;

    QJsonArray metadata_array_;

    int to_flash_electronics_type_;
    int to_flash_hardware_type_;
};

#endif // METADATAHANDLER_HPP

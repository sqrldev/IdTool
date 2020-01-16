/*
 * This file is part of the "IdTool" utility app.
 *
 * MIT License
 *
 * Copyright (c) 2019 Alexander Hauser
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "identityparser.h"

/*!
 *
 * \class IdentityParser
 * \brief A class for parsing identity information stored in the S4 format.
 *
 * \c IdentityParser offers methods for parsing identity data (binary
 * or base64-encoded) and converting the raw data into an \c IdentityModel
 * object.
 *
 * \c IdentityParser does not employ simple static parsing of the S4 format.
 * Instead, it uses json templates to dynamically parse identity blocks.
 * IdTool comes with pre-built json templates for all currently known block
 * types (Types 1, 2 and 3).
 *
 * To create custom block types, look at the standard templates provided
 * with the app (within the blockdev/ subdirectory) to get a sense for how
 * data types, repitition and dynamic item lengths are handled.
 *
 * More information SQRL's storage format can be found here:
 * https://www.grc.com/sqrl/SQRL_Cryptography.pdf
 *
 * \sa IdentityModel
 *
*/

const QString IdentityParser::HEADER = "sqrldata";
const QString IdentityParser::HEADER_BASE64 = "SQRLDATA";

/*!
 * Parses the file specified by \a fileName, and places a pointer to
 * the resulting \c IdentiyModel object into \a model.
 *
 * \throws A \c std::invalid_argument error is thrown if either
 * \a fileName or \a model are invalid. A \c std::runtime_error
 * is thrown if the specified file cannot be read or if parsing
 * failed.
 */

void IdentityParser::parseFile(QString fileName, IdentityModel* model)
{
    if (fileName.isEmpty() || !model)
    {
        throw std::invalid_argument(
                    QObject::tr("Both filename and model must be valid arguments!")
                    .toStdString());
    }

    QFile identityFile(fileName);

    if (!identityFile.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error(QObject::tr("Error reading identity file!")
                .toStdString());
    }

    QByteArray ba = identityFile.readAll();
    identityFile.close();

    parseIdentityData(ba, model);
}

/*!
 * Parses \a identityString, and places a pointer to the resulting
 * \c IdentiyModel object into \a model.
 *
 * This is especially useful for parsing a base64-encoded identity string.
 *
 * \throws A \c std::invalid_argument error is thrown if either
 * \a identityString is empty or \a model is invalid. A \c std::runtime_error
 * is thrown if the parsing failed.
 */

void IdentityParser::parseString(QString identityString, IdentityModel* model)
{
    if (identityString.isEmpty() || !model)
    {
        throw std::invalid_argument(
                    QObject::tr("Both identityText and model must be valid arguments!")
                    .toStdString());
    }

    QByteArray ba = identityString.toLocal8Bit();
    parseIdentityData(ba, model);
}

/*!
 * Checks whether a json block definition exists for the given
 * \a blockType.
 *
 * Returns \c true if a definition for \a blockType exists, and
 * \c false otherwise.
 */

bool IdentityParser::hasBlockDefinition(int blockType)
{
    QDir path = QDir::currentPath();
    QDir fullPath = path.filePath(QString("blockdef/") + QString::number(blockType) + ".json");
    QString sFullPath = fullPath.absolutePath();
    QFile file(sFullPath);

    return file.exists();
}

/*!
 * Parses the raw data specified by \a data, and places a pointer to
 * the resulting \c IdentiyModel object into \a model.
 *
 * \throws A \c std::runtime_error is thrown if parsing of the data failed.
 */

void IdentityParser::parseIdentityData(QByteArray data, IdentityModel* model)
{
    m_bIsBase64 = false;

    if (!checkHeader(data))
    {
        throw std::runtime_error(QObject::tr("Invalid header!")
                                 .toStdString());
    }

    if (m_bIsBase64) data = base64DecodeIdentity(data);
    data = data.mid(HEADER.length()); // skip header

    while (data.length() > 0)
    {
        if (data.length() < 4)
        {
            throw std::runtime_error(
                        QObject::tr("Not enough data in block to read length and type!")
                                     .toStdString());
        }
        int blockLength = getBlockLength(data);
        int blockType = getBlockType(data);

        QByteArray baBlockDef = getBlockDefinitionBytes(blockType);
        if (baBlockDef.isNull() || baBlockDef.isEmpty())
        {
            baBlockDef = getUnknownBlockDefinition();
        }


        QJsonDocument blockDef;
        if(parseBlockDefinition(baBlockDef, &blockDef))
        {
            IdentityBlock block = parseBlock(data, &blockDef);
            model->blocks.push_back(block);
        }

        data = data.mid(blockLength);
    }
}

/*!
 * Base64-decodes \a data and returns the decoded binary identity.
 *
 * \throws A \c std::runtime_error is thrown if \a data is invalid.
 */

QByteArray IdentityParser::base64DecodeIdentity(QByteArray data)
{
    // base64url-illegal line ending and whitespace characters —
    // CR, LF, TAB and SPACE — should be silently
    // ignored for line wrap tolerance.

    QByteArray cleanData;
    for (int i=0; i<data.length(); i++)
    {
        if (static_cast<unsigned char>(data[i]) != 9 ||  // TAB
            static_cast<unsigned char>(data[i]) != 10 || // LF
            static_cast<unsigned char>(data[i]) != 13 || // CR
            static_cast<unsigned char>(data[i]) != 32)   // SPACE
        {
            cleanData.append(static_cast<char>(data[i]));
        }
    }

    QByteArray withoutHeader = cleanData.mid(HEADER.length());
    QByteArray decodedData = QByteArray::fromBase64(
                withoutHeader,
                QByteArray::OmitTrailingEquals |
                QByteArray::Base64UrlEncoding);

    if (decodedData.isEmpty())
    {
        throw std::runtime_error(
                    QObject::tr("Invalid base64-format on identity!")
                    .toStdString());
    }

    return (HEADER.toLocal8Bit() + decodedData);
}

/*!
 * Parses a single binary identity block provided within \a data,
 * using the json block definition \a blockDef as a parsing template,
 * and returns the resulting \c IdentityBlock object.
 *
 * \throws A \c std::runtime_error is thrown if the parsing failed.
 */

IdentityBlock IdentityParser::parseBlock(QByteArray data, QJsonDocument* blockDef)
{
    IdentityBlock newBlock;
    int index = 0;
    auto bd = (*blockDef).object();

    newBlock.blockType = bd["block_type"].toInt();
    newBlock.description = bd["description"].toString();
    newBlock.color = bd["color"].toString();

    QJsonArray items = bd["items"].toArray();

    for (int i=0; i<items.size(); i++)
    {
        IdentityBlockItem newItem;
        QJsonObject item = items.at(i).toObject();

        int repeat_count = 1;
        if (item.contains("repeat_index"))
        {
            int repeat_index = item["repeat_index"].toInt();
            if (newBlock.items.size() > repeat_index)
            {
                repeat_count = newBlock.items[repeat_index].value.toInt();
            }
        }
        else if (item.contains("repeat_count"))
        {
            repeat_count = item["repeat_count"].toInt(1);
        }

        newItem.name = item["name"].toString();
        newItem.description = item["description"].toString();
        newItem.dataType = IdentityBlockItem::findDataType(item["type"].toString());
        newItem.nrOfBytes = item["bytes"].toInt();

        for (int j=0; j<repeat_count; j++)
        {
            if (newItem.dataType == UINT_8)
            {
                if (newItem.nrOfBytes != 1) throw std::runtime_error(
                            QObject::tr("Invalid byte count for datatype UINT_8!")
                            .toStdString());
                newItem.value = parseUint8(data, index);
            }
            else if (newItem.dataType == UINT_16)
            {
                if (newItem.nrOfBytes != 2) throw std::runtime_error(
                            QObject::tr("Invalid byte count for datatype UINT_16!")
                            .toStdString());
                newItem.value = parseUint16(data, index);
            }
            else if (newItem.dataType == UINT_32)
            {
                if (newItem.nrOfBytes != 4) throw std::runtime_error(
                            QObject::tr("Invalid byte count for datatype UINT_32!")
                            .toStdString());
                newItem.value = parseUint32(data, index);
            }
            else if (newItem.dataType == BYTE_ARRAY)
            {
                // If nrOfBytes is set to -1, we shall use all the remaining bytes in the block
                if (newItem.nrOfBytes < 0)
                {
                    if (newBlock.items.size() > 0 &&
                            newBlock.items.at(0).name.toLower() == "length")
                    {
                        newItem.nrOfBytes = newBlock.items.at(0).value.toInt() - index;
                    }
                }
                newItem.value = parseByteArray(data, index, newItem.nrOfBytes);
            }
            else
            {
                item["value"] = "";
            }

            newBlock.items.push_back(newItem);
            index += newItem.nrOfBytes;
        }
    }

    return newBlock;
}

/*!
 * Checks whether a valid SQRL identity header is present within
 * the identity data provided in \a data.
 *
 * Valid identity headers are either "sqrldata", signalling
 * binary identity data, or "SQRLDATA", signalling base64-
 * encoded identity data following immediately after the
 * header.
 *
 * Returns \c true if a valid header was found at the beginning
 * of \a data, or \c false otherwise.
 */

bool IdentityParser::checkHeader(QByteArray data)
{
    if (data.length() < HEADER.length())
    {
        return false;
    }

    QString header(data.left(HEADER.length()));

    if (header != HEADER)
    {
        if (header == HEADER_BASE64)
        {
            this->m_bIsBase64 = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

/*!
 * \brief Gets the raw, binary data for a json block definition for a block
 * of type \a blockType.
 *
 * Searches the "/blockdef" subdirectory for a block definition file named
 * <blockType>.json and returns its contents. If the file does not exists,
 * an empty byte array is returned.
 */

QByteArray IdentityParser::getBlockDefinitionBytes(int blockType)
{
    QDir path = QDir::currentPath();
    QDir fullPath = path.filePath(QString("blockdef/") + QString::number(blockType) + ".json");
    QString sFullPath = fullPath.absolutePath();

    QByteArray ba;
    QFile file(sFullPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
    {
        return ba;
    }
    ba = file.readAll();
    file.close();

    return ba;
}

/*!
 * Parses the raw, binary json block definition provided in \a data,
 * creates a \c QJsonDocument object, and stores a pointer to it in \a jsonDoc .
 *
 * Returns \c true if parsing succeeded, and \c false otherwise.
 */

bool IdentityParser::parseBlockDefinition(QByteArray data, QJsonDocument* jsonDoc)
{
    QJsonParseError error;
    *jsonDoc = QJsonDocument::fromJson(data, &error);

    if (error.error == QJsonParseError::NoError)
    {
        return true;
    }

    return false;
}

/*!
 * Converts \a item into a number of \c QStandardItem objects and returns
 * a list of pointers to those objects.
 *
 * This is used to display block items in a UI view using the view/model paradigm.
 */

QList<QStandardItem*> IdentityParser::toStandardItems(QJsonObject *item)
{
    QList<QStandardItem*> result;

    int nrOfBytes = (*item)["bytes"].toInt(0);
    int repeatIndex = (*item)["repeat_index"].toInt(-1);
    int repeatCount = (*item)["repeat_count"].toInt(1);

    result.append(new QStandardItem((*item)["name"].toString("")));
    result.append(new QStandardItem((*item)["description"].toString("")));
    result.append(new QStandardItem((*item)["type"].toString("UNDEFINED")));
    result.append(new QStandardItem(QString::number(nrOfBytes)));
    result.append(new QStandardItem(QString::number(repeatIndex)));
    result.append(new QStandardItem(QString::number(repeatCount)));

    return result;
}

/*!
 * Returns the raw, binary json block definition for an "unknown block".
 * This block definition is embedded into the application binary and is
 * being used as a fallback if no "specialized" block definition can be
 * found within the "blockdef/" subdirectory for a specific block type.
 *
 * The "unknown block" definition only defines the length, type and raw
 * data for an identity block.
 */

QByteArray IdentityParser::getUnknownBlockDefinition()
{
    QByteArray ba;
    QString resFile = ":/res/file/unknown_blockdef.json";
    QFile file(resFile);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error(
                    QObject::tr("Error accessing resource file for unknown block definition!")
                    .toStdString());
    }
    ba = file.readAll();
    file.close();

    return ba;
}

/*!
 * Creates an \c IdentityBlock object of the given \a blockType,
 * with all item values left blank or undefined, and returns it.
 *
 * If no valid block definition can be found for \a blockType,
 * an empty \c IdentityBlock object is returned.
 */

IdentityBlock IdentityParser::createEmptyBlock(int blockType)
{
    IdentityBlock result;

    QByteArray jsonData = getBlockDefinitionBytes(blockType);
    if (jsonData.isNull() || jsonData.isEmpty()) return result;

    QJsonParseError error;
    QJsonDocument blockDef = QJsonDocument::fromJson(jsonData, &error);

    if (error.error != QJsonParseError::NoError) return result;

    QJsonObject json = blockDef.object();

    if (json.isEmpty()) return result;

    result.blockType = json["block_type"].toInt(-1);
    result.description = json["description"].toString("");
    result.color = json["color"].toString("rgb(0,0,0)");

    QJsonArray jsonItems = json["items"].toArray();

    foreach (QJsonValue jsonItem, jsonItems)
    {
        QJsonObject jsonItemObj = jsonItem.toObject();

        IdentityBlockItem item;
        item.name = jsonItemObj["name"].toString("");
        item.description = jsonItemObj["description"].toString("");
        item.dataType = IdentityBlockItem::findDataType(jsonItemObj["type"].toString("UNDEFINED"));
        item.nrOfBytes = jsonItemObj["name"].toInt(1);
        item.value = "";

        if (jsonItemObj.contains("repeat_index"))
            item.repeatIndex = jsonItemObj["repeat_index"].toInt(-1);

        if (jsonItemObj.contains("repeat_count") &&
                jsonItemObj["repeat_count"].toInt(1) > 1)
            item.repeatCount = jsonItemObj["repeat_count"].toInt(1);

        result.items.push_back(item);
    }

    return result;
}

/*!
 * Creates and returns an \c IdentityBlockItem object with the
 * provided \a name, \a description, \a dataType and \a nrOfBytes.
 */

IdentityBlockItem IdentityParser::createEmptyItem(QString name, QString description,
                                                  ItemDataType dataType, int nrOfBytes)
{
    IdentityBlockItem item;
    item.name = name;
    item.description = description;
    item.dataType = dataType;
    item.nrOfBytes = nrOfBytes;

    return item;
}

/*!
 * Gets the block length, stored within the first two bytes
 * of \a data, and returns it.
 *
 * If \a data holds less than two bytes, \c 0  is returned.
 */

int IdentityParser::getBlockLength(QByteArray data)
{
    if (data.count() < 2) return 0;

    unsigned char c1 = static_cast<unsigned char>(data[0]);
    unsigned char c2 = static_cast<unsigned char>(data[1]);
    return c1 | (c2 << 8);
}

/*!
 * Gets the block type, stored within the third and fourth
 * byte of \a data, and returns it.
 *
 * \throws A \c std::invalid_argument is thrown if \a data
 * holds less than four bytes.
 */

int IdentityParser::getBlockType(QByteArray data)
{
    if (data.count() < 4)
    {
        throw std::invalid_argument(
                    QObject::tr("The provided byte array is too short!")
                    .toStdString());
    }

    unsigned char c1 = static_cast<unsigned char>(data[2]);
    unsigned char c2 = static_cast<unsigned char>(data[3]);
    return c1 | (c2 << 8);
}

/*!
 * Converts the byte stored in \a data at offset \a offset into
 * a string and returns it.
 *
 * \throws A \c std::invalid_argument error is thrown
 * if \c {data.count() <= offset}.
 */

QString IdentityParser::parseUint8(QByteArray data, int offset)
{
    if (data.count() <= offset)
    {
        throw std::invalid_argument(
                    QObject::tr("The provided byte array is too short!")
                    .toStdString());
    }

    return QString::number(static_cast<unsigned char>(data[offset]));
}

/*!
 * Converts the uint16 stored as two bytes within \a data at offset
 * \a offset into a string and returns it.
 *
 * \throws A \c std::invalid_argument error is thrown
 * if \c {data.count() <= offset + 1}.
 */

QString IdentityParser::parseUint16(QByteArray data, int offset)
{
    if (data.count() <= offset + 1)
    {
        throw std::invalid_argument(
                    QObject::tr("The provided byte array is too short!")
                    .toStdString());
    }

    unsigned char c1 = static_cast<unsigned char>(data[offset]);
    unsigned char c2 = static_cast<unsigned char>(data[offset+1]);
    uint32_t num = static_cast<uint32_t>(c1 | (c2 << 8));
    return QString::number(num);
}

/*!
 * Converts the uint32 stored as four bytes within \a data at offset
 * \a offset into a string and returns it.
 *
 * \throws A \c std::invalid_argument error is thrown
 * if \c {data.count() <= offset + 3}.
 */

QString IdentityParser::parseUint32(QByteArray data, int offset)
{
    if (data.count() <= offset + 3)
    {
        throw std::invalid_argument(
                    QObject::tr("The provided byte array is too short!")
                    .toStdString());
    }

    unsigned char c1 = static_cast<unsigned char>(data[offset]);
    unsigned char c2 = static_cast<unsigned char>(data[offset+1]);
    unsigned char c3 = static_cast<unsigned char>(data[offset+2]);
    unsigned char c4 = static_cast<unsigned char>(data[offset+3]);
    uint32_t num = static_cast<uint32_t>(c1 | (c2 << 8) | (c3 << 16) | (c4 << 24));
    return QString::number(num);
}

/*!
 * Converts the binary data within \a data at offset \a offset and with a
 * length of \a bytes number of bytes into a hex-encoded string and returns it.
 *
 * \throws A \c std::invalid_argument error is thrown
 * if \c {data.count() <= offset + bytes - 1}.
 */

QString IdentityParser::parseByteArray(QByteArray data, int offset, int bytes)
{    
    if (data.count() <= offset + bytes - 1)
    {
        throw std::invalid_argument(
                    QObject::tr("The provided byte array is too short!")
                    .toStdString());
    }

    return QString::fromLocal8Bit(data.mid(offset, bytes).toHex());
}

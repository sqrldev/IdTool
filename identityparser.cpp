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

const QString IdentityParser::HEADER = "sqrldata";
const QString IdentityParser::HEADER_BASE64 = "SQRLDATA";

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

void IdentityParser::parseText(QString identityText, IdentityModel* model)
{
    if (identityText.isEmpty() || !model)
    {
        throw std::invalid_argument(
                    QObject::tr("Both filename and model must be valid arguments!")
                    .toStdString());
    }

    QByteArray ba = identityText.toLocal8Bit();
    parseIdentityData(ba, model);
}

bool IdentityParser::hasBlockDefinition(uint16_t blockType)
{
    QDir path = QDir::currentPath();
    QDir fullPath = path.filePath(QString("blockdef/") + QString::number(blockType) + ".json");
    QString sFullPath = fullPath.absolutePath();
    QFile file(sFullPath);

    return file.exists();
}

void IdentityParser::parseIdentityData(QByteArray data, IdentityModel* model)
{
    m_bIsBase64 = false;

    if (!checkHeader(data))
    {
        throw std::runtime_error(QObject::tr("Invalid header!")
                                 .toStdString());
    }

    if (m_bIsBase64)
    {
        data = base64DecodeIdentity(data);
    }

    data = data.mid(HEADER.length()); // skip header

    while (data.length() > 0)
    {
        if (data.length() < 4)
        {
            throw std::runtime_error(
                        QObject::tr("Not enough data in block to read length and type!")
                                     .toStdString());
        }
        uint16_t blockLength = getBlockLength(data);
        uint16_t blockType = getBlockType(data);

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
            size_t repeat_index = static_cast<size_t>(item["repeat_index"].toInt());
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

QByteArray IdentityParser::getBlockDefinitionBytes(uint16_t blockType)
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

IdentityBlock IdentityParser::createEmptyBlock(uint16_t blockType)
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


uint16_t IdentityParser::getBlockLength(QByteArray data)
{
    unsigned char c1 = static_cast<unsigned char>(data[0]);
    unsigned char c2 = static_cast<unsigned char>(data[1]);
    return static_cast<uint16_t>(c1 | (c2 << 8));
}

uint16_t IdentityParser::getBlockType(QByteArray data)
{
    unsigned char c1 = static_cast<unsigned char>(data[2]);
    unsigned char c2 = static_cast<unsigned char>(data[3]);
    return static_cast<uint16_t>(c1 | (c2 << 8));
}

QString IdentityParser::parseUint8(QByteArray data, int offset)
{
    return QString::number(static_cast<unsigned char>(data[offset]));
}

QString IdentityParser::parseUint16(QByteArray data, int offset)
{
    unsigned char c1 = static_cast<unsigned char>(data[offset]);
    unsigned char c2 = static_cast<unsigned char>(data[offset+1]);
    uint32_t num = static_cast<uint32_t>(c1 | (c2 << 8));
    return QString::number(num);
}

QString IdentityParser::parseUint32(QByteArray data, int offset)
{
    unsigned char c1 = static_cast<unsigned char>(data[offset]);
    unsigned char c2 = static_cast<unsigned char>(data[offset+1]);
    unsigned char c3 = static_cast<unsigned char>(data[offset+2]);
    unsigned char c4 = static_cast<unsigned char>(data[offset+3]);
    uint32_t num = static_cast<uint32_t>(c1 | (c2 << 8) | (c3 << 16) | (c4 << 24));
    return QString::number(num);
}

QString IdentityParser::parseByteArray(QByteArray data, int offset, int bytes)
{    
    return QString::fromLocal8Bit(data.mid(offset, bytes).toHex());
}

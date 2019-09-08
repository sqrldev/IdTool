#include "identityparser.h"

const QString IdentityParser::HEADER = "sqrldata";
const QString IdentityParser::HEADER_BASE64 = "SQRLDATA";

void IdentityParser::parseFile(QString fileName, IdentityModel* model)
{
    if (fileName.isEmpty() || !model)
    {
        throw std::invalid_argument("Both filename and model must be valid arguments!");
    }

    QFile identityFile(fileName);
    if (!identityFile.open(QIODevice::ReadOnly)) throw std::runtime_error("Error reading identity file!");
    QByteArray ba = identityFile.readAll();
    identityFile.close();

    parse(ba, model);
}

void IdentityParser::parseText(QByteArray identityText, IdentityModel* model)
{
    if (identityText.isEmpty() || !model)
    {
        throw std::invalid_argument("Both filename and model must be valid arguments!");
    }

    parse(identityText, model);
}

void IdentityParser::parse(QByteArray data, IdentityModel* model)
{
    if (!checkHeader(data))
    {
        throw std::runtime_error("Invalid header!");
    }

    if (m_bIsBase64)
    {
        QString decoded = QString::fromLocal8Bit(data.toBase64());
        if (decoded.isEmpty()) throw std::runtime_error("Invalid base64-format on identity!");

        decoded = HEADER + decoded;
        data = decoded.toLocal8Bit().data();
    }

    int i = HEADER.length(); // skip header

    while (i < data.length())
    {
        uint16_t blockLength = getBlockLength(&(data.data()[i]));
        uint16_t blockType = getBlockType(&(data.data()[i]));

        QByteArray sBlockDef = getBlockDefinition(blockType);
        QJsonDocument blockDef = QJsonDocument::fromJson(sBlockDef, nullptr);

        IdentityModel::IdentityBlock block = parseBlock(&(data.data()[i]), &blockDef);
        model->blocks.push_back(block);

        i += blockLength;
    }
}

IdentityModel::IdentityBlock IdentityParser::parseBlock(const char* data, QJsonDocument* blockDef)
{
    IdentityModel::IdentityBlock newBlock;
    int index = 0;
    auto bd = (*blockDef);

    newBlock.blockType = bd["block_type"].toInt();
    newBlock.description = bd["description"].toString();
    newBlock.color = bd["color"].toString();

    QJsonArray items = bd["items"].toArray();

    for (int i=0; i<items.size(); i++)
    {
        IdentityModel::IdentityBlockItem newItem;
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

        newItem.name = item["name"].toString();
        newItem.description = item["description"].toString();
        newItem.type = item["type"].toString();
        newItem.bytes = item["bytes"].toInt();

        for (int j=0; j<repeat_count; j++)
        {
            if (newItem.type == "UINT_8")
            {
                if (newItem.bytes != 1) throw std::runtime_error("Invalid byte count for datatype UINT_8!");
                newItem.value = parseUint8(data, index);
            }
            else if (newItem.type == "UINT_16")
            {
                if (newItem.bytes != 2) throw std::runtime_error("Invalid byte count for datatype UINT_16!");
                newItem.value = parseUint16(data, index);
            }
            else if (newItem.type == "UINT_32")
            {
                if (newItem.bytes != 4) throw std::runtime_error("Invalid byte count for datatype UINT_32!");
                newItem.value = parseUint32(data, index);
            }
            else if (newItem.type == "BYTE_ARRAY")
            {
                if (newItem.bytes < 1) throw std::runtime_error("Invalid byte count for datatype BYTE_ARRAY!");
                newItem.value = parseByteArray(data, index, newItem.bytes);
            }
            else
            {
                item["value"] = "";
            }

            newBlock.items.push_back(newItem);
            index += newItem.bytes;
        }
    }

    return newBlock;
}

bool IdentityParser::checkHeader(QByteArray data)
{
    QString header(data);
    header = header.left(HEADER.length());

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

QByteArray IdentityParser::getBlockDefinition(uint16_t blockType)
{
    QDir path = QDir::currentPath();
    QDir fullPath = path.filePath(QString("blockdef/") + QString::number(blockType) + ".json");
    QString sFullPath = fullPath.absolutePath();

    QFile file(sFullPath);
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Error reading block definition file!");
    QByteArray ba = file.readAll();
    file.close();

    return ba;
}

uint16_t IdentityParser::getBlockLength(const char* data)
{
    return static_cast<uint16_t>(data[0] | (data[1] << 8));
}

uint16_t IdentityParser::getBlockType(const char* data)
{
    return static_cast<uint16_t>(data[2] | (data[3] << 8));
}

QString IdentityParser::parseUint8(const char* data, int offset)
{
    return QString::number(static_cast<uint8_t>(data[offset]));
}

QString IdentityParser::parseUint16(const char* data, int offset)
{
    return QString::number(static_cast<uint16_t>(data[offset] | (data[offset+1] << 8)));
}

QString IdentityParser::parseUint32(const char* data, int offset)
{
    uint32_t num = static_cast<uint32_t>(data[offset] | (data[offset+1] << 8) | (data[offset+2] << 16) | (data[offset+3] << 24));
    return QString::number(num);
}

QString IdentityParser::parseByteArray(const char* data, int offset, int bytes)
{
    QByteArray ba(&data[offset], bytes);
    return QString::fromLocal8Bit(ba.toBase64());
}
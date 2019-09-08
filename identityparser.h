#ifndef S4PARSER_H
#define S4PARSER_H

#include "json.hpp"
#include "base64.h"
#include "identitymodel.h"
#include <fstream>
#include <QDir>

using json = nlohmann::json;

class IdentityParser
{
public:
    const char* HEADER = "sqrldata";
    const char* HEADER_BASE64 = "SQRLDATA";
    const size_t HEADER_SIZE = strlen(HEADER);

private:
    bool m_bIsBase64 = false;

public:
    IdentityParser(){}

    void parseFile(const char* fileName, IdentityModel* model)
    {
        if (!fileName || !model)
        {
            throw std::invalid_argument("Both filename and model must be valid pointers!");
        }

        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        std::streamsize size = static_cast<std::streamsize>(file.tellg());
        file.seekg(0, std::ios::beg);

        char buffer[size];
        if (!file.read(buffer, size))
        {
            throw std::runtime_error("Error reading the identity file!");
        }

        file.close();
        parse(buffer, static_cast<size_t>(size), model);
    }

    void parseText(const char* identityText, IdentityModel* model)
    {
        if (!identityText || !model)
        {
            throw std::invalid_argument("Both filename and model must be valid pointers!");
        }

        size_t len = strlen(identityText);
        parse(identityText, len, model);
    }


private:
    void parse(const char *data, size_t length, IdentityModel* model)
    {
        const char* myData = data;

        if (!checkHeader(myData))
        {
            throw std::runtime_error("Invalid header!");
        }

        if (m_bIsBase64)
        {
            std::string decoded = base64_decode(std::string(myData, length));
            if (decoded.empty()) throw std::runtime_error("Invalid base64-format on identity!");

            decoded = std::string(HEADER, HEADER_SIZE) + decoded;
            myData = decoded.c_str();
            length = decoded.length();
        }

        size_t i = HEADER_SIZE; // skip header

        while (i < length)
        {
            uint16_t blockLength = getBlockLength(&myData[i]);
            uint16_t blockType = getBlockType(&myData[i]);

            std::string sBlockDef = getBlockDefinition(blockType);
            json blockDef = json::parse(sBlockDef);

            IdentityModel::IdentityBlock block = parseBlock(&myData[i], &blockDef);
            model->blocks.push_back(block);

            i += blockLength;
        }
    }

    IdentityModel::IdentityBlock parseBlock(const char* data, json* blockDefinition)
    {
        IdentityModel::IdentityBlock newBlock;
        int index = 0;
        auto items = (*blockDefinition)["items"];

        newBlock.blockType = (*blockDefinition)["block_type"];
        newBlock.description = (*blockDefinition)["description"];
        newBlock.color = (*blockDefinition)["color"];

        for (size_t i=0; i<items.size(); i++)
        {
            IdentityModel::IdentityBlockItem newItem;
            auto item = items.at(i);

            int repeat_count = 1;
            if (item.contains("repeat_index"))
            {
                size_t repeat_index = item["repeat_index"];
                if (newBlock.items.size() > repeat_index)
                {
                    repeat_count = atoi(newBlock.items[repeat_index].value.c_str());
                }
            }

            newItem.name = item["name"];
            newItem.description = item["description"];
            newItem.type = item["type"];
            newItem.bytes = item["bytes"];

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

    bool checkHeader(const char* data)
    {
        char header[HEADER_SIZE + 1];
        memset(header, 0, HEADER_SIZE + 1);
        memcpy(header, data, HEADER_SIZE);

        if (strcmp(HEADER, header) != 0)
        {
            if (strcmp(HEADER_BASE64, header) == 0)
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

    std::string getBlockDefinition(uint16_t blockType)
    {
        QDir path = QDir::currentPath();
        QDir fullPath = path.filePath(QString("blockdef/") + QString::number(blockType) + ".json");
        std::string fileName = fullPath .absolutePath().toStdString();

        std::ifstream t(fileName);
        std::string contents((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
        return contents;
    }

    uint16_t getBlockLength(const char* data)
    {
        return static_cast<uint16_t>(data[0] | (data[1] << 8));
    }

    uint16_t getBlockType(const char* data)
    {
        return static_cast<uint16_t>(data[2] | (data[3] << 8));
    }

    std::string parseUint8(const char* data, int offset)
    {
        return std::to_string(data[offset]);
    }

    std::string parseUint16(const char* data, int offset)
    {
        return std::to_string(data[offset] | (data[offset+1] << 8));
    }

    std::string parseUint32(const char* data, int offset)
    {
        uint32_t num = static_cast<uint32_t>(data[offset] | (data[offset+1] << 8) | (data[offset+2] << 16) | (data[offset+3] << 24));
        return std::to_string(num);
    }

    std::string parseByteArray(const char* data, int offset, int bytes)
    {
        return base64_encode(reinterpret_cast<const unsigned char*>(&data[offset]), static_cast<unsigned int>(bytes));
    }
};

#endif // S4PARSER_H

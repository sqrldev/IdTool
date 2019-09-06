#ifndef S4PARSER_H
#define S4PARSER_H

#include "json.hpp"
#include "base64.h"
#include "identitymodel.h"
#include <fstream>
#include <QDir>

using json = nlohmann::json;

class S4Parser
{
public:
    const char* HEADER = "sqrldata";
    const char* HEADER_BASE64 = "SQRLDATA";
    const size_t HEADER_SIZE = strlen(HEADER);

private:
    bool m_bIsBase64 = false;

public:
    S4Parser(){}

    void parseIdentityFile(const char* fileName, IdentityModel* model)
    {
        if (!fileName)
        {
            throw std::invalid_argument("Both fileName and model must be valid pointers!");
        }

        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        char buffer[size];
        if (!file.read(buffer, size))
        {
            throw std::runtime_error("Error reading the identity file!");
        }

        file.close();
        parseIdentity(buffer, static_cast<size_t>(size), model);
    }

    void parseIdentityText(const char* identityText, IdentityModel* model)
    {
        if (!identityText)
        {
            throw std::invalid_argument("Both fileName and model must be valid pointers!");
        }

        size_t len = strlen(identityText);
        parseIdentity(identityText, len, model);
    }


private:
    void parseIdentity(const char *data, size_t length, IdentityModel* model)
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
            json* pBlockDefinition = getBlockDefinition(blockType);
            parseBlock(&myData[i], pBlockDefinition);
            model->blocks.push_back(1);

            i += blockLength;
        }
    }

    void parseBlock(const char* data, json* blockDefinition)
    {
        int t = (*blockDefinition)["block_type"];
        if (t != 1) return;

        size_t nrOfItems = (*blockDefinition)["items"].size();

        for (size_t i=0; i<nrOfItems; i++)
        {
            std::string sType = (*blockDefinition)["items"].at(i)["type"];
            int bytes =  (*blockDefinition)["items"].at(i)["bytes"];

            if (sType == "UINT_8")
            {
                if (bytes != 1) throw std::runtime_error("Invalid byte count for datatype UINT_8!");
                (*blockDefinition)["items"].at(i)["value"] = parseUint8(data, 0);
            }
            else if (sType == "UINT_16")
            {
                if (bytes != 2) throw std::runtime_error("Invalid byte count for datatype UINT_16!");
                (*blockDefinition)["items"].at(i)["value"] = parseUint16(data, 0);
            }
            else if (sType == "UINT_32")
            {
                if (bytes != 4) throw std::runtime_error("Invalid byte count for datatype UINT_32!");
                (*blockDefinition)["items"].at(i)["value"] = parseUint32(data, 0);
            }
            else if (sType == "BYTE_ARRAY")
            {
                if (bytes < 1) throw std::runtime_error("Invalid byte count for datatype BYTE_ARRAY!");
                (*blockDefinition)["items"].at(i)["value"] = 1; //parseByteArray(data, 0, bytes);
            }
            else
            {
                (*blockDefinition)["items"].at(i)["value"] = 1; //"";
            }
        }
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

    uint16_t getBlockLength(const char* data)
    {
        return static_cast<uint16_t>(data[0] | (data[1] << 8));
    }

    uint16_t getBlockType(const char* data)
    {
        return static_cast<uint16_t>(data[2] | (data[3] << 8));
    }

    json* getBlockDefinition(uint16_t blockType)
    {
        QDir path = QDir::currentPath();
        path.filePath("blockdef/");
        QDir fullPath = path.filePath(QString::number(blockType) + ".json");
        std::string fileName = fullPath.absolutePath().toStdString();

        std::ifstream t("C:\\Users\\alex.hauser\\Source\\Repos\\IdTool\\blockdef\\1.json");
        std::string contents((std::istreambuf_iterator<char>(t)),
                         std::istreambuf_iterator<char>());
        return new json(contents);
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
        std::to_string(data[offset] | (data[offset+1] << 8) | (data[offset+2] << 16) | (data[offset+3] << 24));
    }

    std::string parseByteArray(const char* data, int offset, int bytes)
    {
        return base64_encode(reinterpret_cast<const unsigned char*>(&data[offset]), static_cast<unsigned int>(bytes));
    }
};

#endif // S4PARSER_H

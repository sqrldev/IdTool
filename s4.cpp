#include "s4.hpp"
#include <fstream>
#include <vector>
#include <cstring>

namespace SqrlStorage
{
    const int S4::HEADER_SIZE = 8;
    const char* S4::HEADER = "sqrldata";
    const char* S4::HEADER_BASE64 = "SQRLDATA";

    void S4::readIdentityFile(char* fileName)
    {
        std::ifstream file(fileName, std::ios::binary | std::ios::ate);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> data(static_cast<unsigned long long>(size));
        if (!file.read(data.data(), size))
        {
            throw std::runtime_error("Error reading the identity file!");
        }

        parse(&data);
    }

    void S4::readIdentityText(char* identityData, int length)
    {
        std::vector<char> data(identityData, identityData + length);
        parse(&data);
    }

    void S4::parse(std::vector<char> *data)
    {
        if (!checkHeader(data))
        {
            throw std::runtime_error("Invalid header!");
        }

        int i = HEADER_SIZE;
        while (i < static_cast<int>(data->size()))
        {
            Block block(data, i);

            switch (block.type)
            {
            case EBlockType::TYPE_1_PW_ENCRYPTED_DATA:
            {
                BlockType1 *pBlock = new BlockType1(data, i);
                this->blocks.push_back(pBlock);
                break;
            }

            case EBlockType::TYPE_2_RC_ENCRYPTED_DATA:
            {
                BlockType2 *pBlock = new BlockType2(data, i);
                this->blocks.push_back(pBlock);
                break;
            }

            case EBlockType::TYPE_3_PREVIOUS_IUK:
            {
                BlockType3 *pBlock = new BlockType3(data, i);
                this->blocks.push_back(pBlock);
                break;
            }

            default:
                this->blocks.push_back(&block);
                break;
            }

            i += block.length;
        }
    }

    bool S4::checkHeader(std::vector<char>* data)
    {
        if (data->size() < HEADER_SIZE) return false;

        char header[HEADER_SIZE + 1] = { 0 };
        memcpy(header, data->data(), HEADER_SIZE);

        if (strcmp(HEADER, header) != 0)
        {
            if (strcmp(HEADER_BASE64, header) == 0)
            {
                this->m_IsBase64 = true;
                return true;
            }
            else
            {
                return false;
            }
        }

        return true;
    }

    S4::~S4()
    {
        for (size_t i=0; i<this->blocks.size(); i++)
        {
            delete this->blocks[i];
        }
        this->blocks.clear();
    }

    S4::Block::Block(std::vector<char>* data, int blockStartIndex)
    {
        if (static_cast<int>(data->size()) < blockStartIndex + 4)
        {
            throw std::invalid_argument("Provided block is too short!");
        }

        char* pData = &(data->data()[blockStartIndex]);

        this->length = static_cast<uint16_t>(pData[0] | (pData[1] << 8));
        this->type = static_cast<uint16_t>(pData[2] | (pData[3] << 8));
    }

    S4::BlockType1::BlockType1(std::vector<char>* data, int blockStartIndex)
        : Block(data, blockStartIndex)
    {
        if (static_cast<int>(data->size()) - blockStartIndex < this->length)
        {
            throw std::invalid_argument("Provided block is too short!");
        }

        char* pData = data->data() + blockStartIndex + 4;

        this->plainTextLength = static_cast<uint16_t>(pData[0] | (pData[1] << 8));
        memcpy(this->aesGcmIv, &pData[2], sizeof(this->aesGcmIv));
        memcpy(this->scryptRandomSalt, &pData[14], sizeof(this->scryptRandomSalt));
        this->scryptLogNFactor = static_cast<unsigned char>(pData[30]);
        this->scryptIterationCount = static_cast<uint32_t>
                (pData[31] | (pData[32] << 8) | (pData[33] << 16) | (pData[34] << 24));
        this->optionFlags = static_cast<uint16_t>(pData[35] | (pData[36] << 8));
        this->hintLength = static_cast<unsigned char>(pData[37]);
        this->pwVerifySecs = static_cast<unsigned char>(pData[38]);
        this->idleTimeout = static_cast<uint16_t>(pData[39] | (pData[40] << 8));
        memcpy(this->identityMasterKey, &pData[41], sizeof(this->identityMasterKey));
        memcpy(this->identityLockKey, &pData[73], sizeof(this->identityLockKey));
        memcpy(this->verificationTag, &pData[105], sizeof(this->verificationTag));
    }

    S4::BlockType2::BlockType2(std::vector<char>* data, int blockStartIndex) :
        Block(data, blockStartIndex)
    {
        if (static_cast<int>(data->size()) - blockStartIndex < this->length)
        {
            throw std::invalid_argument("Provided block is too short!");
        }

        char* pData = data->data() + blockStartIndex + 4;

        memcpy(this->scryptRandomSalt, &pData[0], sizeof(this->scryptRandomSalt));
        this->scryptLogNFactor = static_cast<unsigned char>(pData[16]);
        this->scryptIterationCount = static_cast<uint32_t>
                (pData[17] | (pData[18] << 8) | (pData[19] << 16) | (pData[20] << 24));
        memcpy(this->identityUnlockKey, &pData[21], sizeof(this->identityUnlockKey));
        memcpy(this->verificationTag, &pData[53], sizeof(this->verificationTag));
    }

    S4::BlockType3::BlockType3(std::vector<char>* data, int blockStartIndex) :
        Block(data, blockStartIndex)
    {
        if (static_cast<int>(data->size()) - blockStartIndex < this->length)
        {
            throw std::invalid_argument("Provided block is too short!");
        }

        char* pData = data->data() + blockStartIndex + 4;

        this->nrOfPreviousIuks = static_cast<uint16_t>(pData[0] | (pData[1] << 8));
        pData += 2;

        for (int i = 1; i <= nrOfPreviousIuks; i++)
        {
            char* pIuk = new char[32]{ 0 };
            memcpy(pIuk, pData, 32);
            this->previousIuks.push_back(pIuk);
            pData += 32;

        }
        memcpy(this->verificationTag, pData, sizeof(this->verificationTag));
    }

    S4::BlockType3::~BlockType3()
    {
        for (unsigned int i = 0; i < this->previousIuks.size(); i++)
        {
            delete[] this->previousIuks[i];
        }
        this->previousIuks.clear();
    }

}


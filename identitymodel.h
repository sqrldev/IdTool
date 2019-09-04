#ifndef IDENTITYMODEL_H
#define IDENTITYMODEL_H

#include <QString>
#include <QApplication>
#include <QTranslator>
#include <vector>

enum EItemDataType
{
    UINT_8,
    UINT_16,
    UINT_32,
    BYTE_ARRAY,
    UNDEFINED
};

class IdentityBlockItem
{
public:
    QString name = "";
    QString description = "";
    EItemDataType type = EItemDataType::UNDEFINED;
    int bytes = -1;
    QString value = "";

    bool parseValue(std::vector<char> data, int offset)
    {
        if (name == "" ||
            description == "" ||
            type == EItemDataType::UNDEFINED ||
            bytes == -1)
        {
            return false;
        }

        if (data.size() < (offset + bytes))
        {
            return false;
        }

        switch (type)
        {
        case EItemDataType::UINT_8:
            if (bytes != 1)
            {
                throw std::runtime_error("Invalid byte count for datatype UINT_8!");
                return false;
            }
            value = parseUint8(data, offset);
            return true;

        case EItemDataType::UINT_16:
            if (bytes != 2) throw std::runtime_error("Invalid byte count for datatype UINT_16!");
            value = parseUint16(data, offset);
            break;

        case EItemDataType::UINT_32:
            if (bytes != 4) throw std::runtime_error("Invalid byte count for datatype UINT_32!");
            value = parseUint32(data, offset);
            break;

        case EItemDataType::BYTE_ARRAY:
            if (bytes < 1) throw std::runtime_error("Invalid byte count for datatype BYTE_ARRAY!");
            value = parseByteArray(data, offset, bytes);
            break;
        }
    }

    QString parseUint8(std::vector<char> data, int offset)
    {
        if ((data.size() - offset) < 1) return "";
        return QString::number(data[offset]);
    }

    QString parseUint16(std::vector<char> data, int offset)
    {
        if ((data.size() - offset) < 2) return "";
        return QString::number(data[offset] | (data[offset+1] << 8));
    }

    QString parseUint32(std::vector<char> data, int offset)
    {
        if ((data.size() - offset) < 4) return "";
        return QString::number(data[offset] | (data[offset+1] << 8) | (data[offset+2] << 16) | (data[offset+3] << 24));
    }

    QString parseByteArray(std::vector<char> data, int offset, int bytes)
    {
        if ((data.size() - offset - bytes) < 1) return "";

        for (int i=offset; i<(offset + bytes); i++)
        {
            //TODO: Base64-encode!!
        }
        return "";
    }
};

class IdentityBlock
{
private:
    std::vector<IdentityBlockItem*> m_Items;

public:
    IdentityBlock();
    ~IdentityBlock()
    {
        for (size_t i=0; i<m_Items.size(); i++)
        {
            delete m_Items[i];
        }
        m_Items.clear();
    }
};

class IdentityModel
{
private:
    std::vector<IdentityBlock*> m_Blocks;

public:
    IdentityModel();
    ~IdentityModel()
    {
        for (size_t i=0; i<m_Blocks.size(); i++)
        {
            delete m_Blocks[i];
        }
        m_Blocks.clear();
    }
};

#endif // IDENTITYMODEL_H

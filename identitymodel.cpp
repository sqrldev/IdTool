#include "identitymodel.h"

void IdentityModel::writeToFile(QString fileName)
{
    QByteArray ba;

    for (size_t i=0; i<blocks.size(); i++)
    {
        ba.append(blocks[i].toByteArray());
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) throw std::runtime_error("Error writing identity file!");
    file.write(ba);
    file.close();
}

QByteArray IdentityModel::IdentityBlock::toByteArray()
{
    QByteArray ba;

    for (size_t i=0; i<items.size(); i++)
    {
        QByteArray value;

        if (items[i].type == "UINT_8" ||
            items[i].type == "UINT_16" ||
            items[i].type == "UINT_32")
        {
            ba.append(QByteArray::number(items[i].value.toInt()));
        }
        else if (items[i].type == "BYTE_ARRAY")
        {
            ba.append(QByteArray::fromBase64(items[i].value.toUtf8()));
        }
    }

    return ba;
}

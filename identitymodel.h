#ifndef IDENTITYMODEL_H
#define IDENTITYMODEL_H

#include <vector>
#include <map>
#include <string>
#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QDebug>

class IdentityModel
{     
public: // Forward declarations

    class IdentityBlock;
    class IdentityBlockItem;

public: // Public member variables
    std::vector<IdentityBlock> blocks;

public: // Public methods
    void writeToFile(QString fileName);
    void deleteBlock(IdentityBlock* block);

public: // Internal classes
    class IdentityBlock
    {
    public: // Public vars
        int blockType = -1;
        QString description = "";
        QString color = "rgb(214, 201, 163)";
        std::vector<IdentityBlockItem> items;

    public: // Public methods
        QByteArray toByteArray();
    };

    class IdentityBlockItem
    {
    public: // Public enums
        enum EDataType
        {
            UINT_8,
            UINT_16,
            UINT_32,
            BYTE_ARRAY,
            STRING,
            UNDEFINED
        };

    public: // Public member variables
        QString name = "";
        QString description = "";
        QString type = "UNDEFINED";
        int bytes = 0;
        QString value = "";
    };
};

#endif // IDENTITYMODEL_H

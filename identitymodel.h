#ifndef IDENTITYMODEL_H
#define IDENTITYMODEL_H

#include <vector>
#include <map>
#include <string>

class IdentityModel
{     
public:
    // Forward declarations
    class IdentityBlock;
    class IdentityBlockItem;

public:
    // Public member variables
    std::vector<IdentityBlock> blocks;

public:
    // Internal classes
    class IdentityBlock
    {
    public:
        int blockType = -1;
        std::string description = "";
        std::vector<IdentityBlockItem> items;

        // C-tor
        IdentityBlock() {}
        // Copy-C-tor
        IdentityBlock(const IdentityBlock &block2)
        {
            blockType = block2.blockType;
            description = block2.description;
            items = block2.items;
        }
    };

    class IdentityBlockItem
    {
    public:
        // Public enums
        enum EDataType
        {
            UINT_8,
            UINT_16,
            UINT_32,
            BYTE_ARRAY,
            STRING,
            UNDEFINED
        };

    public:
        // Public member variables
        std::string name = "";
        std::string description = "";
        std::string type = "UNDEFINED";
        int bytes = 0;
        std::string value = "";

    public:
        // C-tor
        IdentityBlockItem() {}
        // Copy-C-tor
        IdentityBlockItem(const IdentityBlockItem &item2)
        {
            name = item2.name;
            description = item2.description;
            type = item2.type;
            bytes = item2.bytes;
            value = item2.value;
        }
    };
};

#endif // IDENTITYMODEL_H

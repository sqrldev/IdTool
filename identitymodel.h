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
        std::string color = "rgb(214, 201, 163)";
        std::vector<IdentityBlockItem> items;
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
    };
};

#endif // IDENTITYMODEL_H

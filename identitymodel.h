#ifndef IDENTITYMODEL_H
#define IDENTITYMODEL_H

#include <vector>
#include <json.hpp>

using json = nlohmann::json;

class IdentityModel
{  
public:
    std::vector<json*> blocks;
    IdentityModel() {}
    ~IdentityModel()
    {
        for (size_t i=0; i<blocks.size(); i++)
        {
            delete blocks[i];
        }
        blocks.clear();
    }

};

#endif // IDENTITYMODEL_H

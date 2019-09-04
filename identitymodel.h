#ifndef IDENTITYMODEL_H
#define IDENTITYMODEL_H

#include <QString>
#include <vector>

class IdentityBlockItem
{
public:
    QString name;
    QString description;
    QString type;
    int bytes;
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

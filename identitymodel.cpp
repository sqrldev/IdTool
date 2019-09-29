/*
 * This file is part of the "IdTool" utility app.
 *
 * MIT License
 *
 * Copyright (c) 2019 Alexander Hauser
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "identitymodel.h"
#include "identityparser.h"

void IdentityModel::writeToFile(QString fileName)
{
    QByteArray ba(IdentityParser::HEADER.toUtf8());

    for (size_t i=0; i<blocks.size(); i++)
    {
        ba.append(blocks[i].toByteArray());
    }

    QFile file(fileName);

    if (!file.open(QFile::WriteOnly))
    {
        throw std::runtime_error(
                QObject::tr("Error opening identity file for writing!")
                    .toStdString());
    }

    file.write(ba);
    file.close();
}

QByteArray IdentityBlock::toByteArray()
{
    QByteArray ba;

    for (size_t i=0; i<items.size(); i++)
    {
        if (items[i].dataType == UINT_8)
        {
            ba.append(static_cast<char>(items[i].value.toInt()));
        }
        else if (items[i].dataType == UINT_16)
        {
            int value = items[i].value.toInt();
            char* data = static_cast<char*>(static_cast<void*>(&value));
            ba.append(data[0]);
            ba.append(data[1]);
        }
        else if (items[i].dataType == UINT_32)
        {
            QString v = items[i].value;
            int vi = v.toInt();
            uint32_t value = static_cast<uint32_t>(vi);
            char* data = static_cast<char*>(static_cast<void*>(&value));
            ba.append(data[0]);
            ba.append(data[1]);
            ba.append(data[2]);
            ba.append(data[3]);
        }
        else if (items[i].dataType == BYTE_ARRAY)
        {
            QByteArray baTemp = QByteArray::fromHex(items[i].value.toUtf8());
            ba.append(baTemp);
        }
    }

    return ba;
}

bool IdentityModel::deleteBlock(IdentityBlock* block)
{
    for (size_t i=0; i<blocks.size(); i++)
    {
        if (&blocks[i] == block)
        {
            blocks.erase(blocks.begin()+ static_cast<long long>(i));
            return true;
        }
    }

    return false;
}

bool IdentityModel::moveBlock(IdentityBlock* block, bool up)
{
    if (blocks.size() < 2) return false;

    for (size_t i=0; i<blocks.size(); i++)
    {
        if (&blocks[i] == block)
        {
            long long swapWith;

            if (up)
            {
                if (i == 0) return false;
                swapWith = static_cast<long long>(i) - 1;
            }
            else
            {
                if (i == (blocks.size() - 1)) return false;
                swapWith = static_cast<long long>(i) + 1;
            }

            iter_swap(blocks.begin() + static_cast<long long>(i), blocks.begin() + swapWith);
            return true;
        }
    }

    return false;
}

bool IdentityModel::insertBlock(IdentityBlock block, IdentityBlock* after)
{
    auto iter = blocks.begin();

    for (size_t i=0; i<blocks.size(); i++)
    {
        if (&blocks[i] == after)
        {
            blocks.insert(iter + 1, block);
            return true;
        }

        iter++;
    }

    return false;
}

void IdentityModel::clear()
{
    blocks.clear();
}

bool IdentityBlock::duplicateItem(IdentityBlockItem* item)
{
    for (auto iter=items.begin(); iter!=items.end(); iter++)
    {
        if (&(*iter) == item)
        {
            IdentityBlockItem tempItem = *iter;
            items.insert(iter, tempItem);
            return true;
        }
    }

    return false;
}

bool IdentityBlock::deleteItem(IdentityBlockItem* item)
{
    for (auto iter=items.begin(); iter!=items.end(); iter++)
    {
        if (&(*iter) == item)
        {
            items.erase(iter);
            return true;
        }
    }

    return false;
}

bool IdentityBlock::moveItem(IdentityBlockItem *item, bool up)
{
    if (items.size() < 2) return false;

    for (size_t i=0; i<items.size(); i++)
    {
        if (&items[i] == item)
        {
            long long swapWith;

            if (up)
            {
                if (i == 0) return false;
                swapWith = static_cast<long long>(i) - 1;
            }
            else
            {
                if (i == (items.size() - 1)) return false;
                swapWith = static_cast<long long>(i) + 1;
            }

            iter_swap(items.begin() + static_cast<long long>(i), items.begin() + swapWith);
            return true;
        }
    }

    return false;
}

bool IdentityBlock::insertItem(IdentityBlockItem item, IdentityBlockItem *after)
{
    auto iter = items.begin();

    for (size_t i=0; i<items.size(); i++)
    {
        if (&items[i] == after)
        {
            items.insert(iter + 1, item);
            return true;
        }

        iter++;
    }

    return false;
}

std::map<ItemDataType, ItemDataTypeInfo> IdentityBlockItem::DataTypeMap = {
    {ItemDataType::UINT_8, {"UINT_8", 1}},
    {ItemDataType::UINT_16, {"UINT_16", 2}},
    {ItemDataType::UINT_32, {"UINT_32", 4}},
    {ItemDataType::BYTE_ARRAY, {"BYTE_ARRAY", 0}},
    {ItemDataType::STRING, {"STRING", 0}},
    {ItemDataType::UNDEFINED, {"UNDEFINED", 0}}
};

ItemDataType IdentityBlockItem::findDataType(QString dataType)
{
    std::map<ItemDataType, ItemDataTypeInfo>::const_iterator it;
    ItemDataType key = ItemDataType::UNDEFINED;

    for (it = DataTypeMap.begin(); it != DataTypeMap.end(); ++it)
    {
        if (it->second.name == dataType)
        {
            key = it->first;
            break;
        }
    }

    return key;
}

QStringList IdentityBlockItem::getDataTypeList()
{
    std::map<ItemDataType, ItemDataTypeInfo>::const_iterator it;
    QStringList result;

    for (it = DataTypeMap.begin(); it != DataTypeMap.end(); ++it)
    {
        result.append(it->second.name);
    }

    return result;
}

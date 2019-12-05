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
#include "cryptutil.h"

/*!
 *
 * \class IdentityModel
 * \brief Represents a SQRL identity stored in the S4-format.
 *
 * \c IdentityModel holds a vector of \c IdentityBlock objects,
 * modelling an object-oriented reproduction of the S4 identity
 * storage format. Following this pattern, \c IdentityBlock
 * holds a vector of \a IdentityBlockItem objects, each representing
 * a single setting/value.
 *
 * More information SQRL's storage format can be found at
 * https://www.grc.com/sqrl/SQRL_Cryptography.pdf
 *
 * \sa IdentityBlock, IdentityBlockItem
 *
*/

/*!
 * Retrieves the raw binary representation of the identity model
 * and writes it to a file specified by \a fileName.
 *
 * \throws A \c std::runtime_error is raised if the file cannot
 * be written.
 */

void IdentityModel::writeToFile(QString fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::WriteOnly))
    {
        throw std::runtime_error(
                QObject::tr("Error opening identity file for writing!")
                    .toStdString());
    }

    QByteArray ba = getRawBytes();
    file.write(ba);
    file.close();
}

/*!
 * Returns the \c IdentityBlock of type \a blockType stored
 * within the identity's list of blocks.
 *
 * If no block of type \a blockType is present, \c nullptr
 * is returned.
 *
 * \sa IdentityBlock
 */

IdentityBlock *IdentityModel::getBlock(uint16_t blockType)
{
    for (size_t i=0; i<blocks.size(); i++)
    {
        if (blocks[i].blockType == blockType)
            return &blocks[i];
    }

    return nullptr;
}

/*!
 * Returns the raw binary representation of the identity block.
 */

QByteArray IdentityBlock::toByteArray()
{
    QByteArray ba;

    for (size_t i=0; i<items.size(); i++)
    {
        ba.append(items[i].toByteArray());
    }

    return ba;
}

/*!
 * Deletes the \c IdentityBlock of type \a blockType from
 * the identity's list of blocks.
 *
 * \return Returns \c true if a block of type \a blockType
 * was found and deleted, and \c false otherwise.
 *
 * \sa IdentityBlock
 */

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

/*!
 * Moves \a block one step upwards or downwards within the identity's
 * list of blocks (imagine a vertical index going from low index (up)
 * to high index (down)).
 *
 * If \a up is \c true, \a block will be moved upwards (index -= 1),
 * or downwards (index += 1) otherwise.
 */

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

/*!
 * Inserts the \c IdentityBlock \a block into the identity's block
 * list, placing it directly after the existing block pointed to
 * by \a after.
 *
 * Returns \c true if \a after was found and \a block was inserted
 * right after it, and \c false if \a after was not found in the list.
 *
 * \sa IdentityBlock
 */

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

/*!
 * Clears the identity's list of blocks.
 */

void IdentityModel::clear()
{
    blocks.clear();
}

/*!
 * Clears the identity and imports all blocks from \a model.
 */

void IdentityModel::import(IdentityModel &model)
{
    clear();
    blocks = model.blocks;
}

/*!
 * Returns the raw binary data representation of the identity.
 */

QByteArray IdentityModel::getRawBytes()
{
    QByteArray ba(IdentityParser::HEADER.toUtf8());

    for (size_t i=0; i<blocks.size(); i++)
    {
        ba.append(blocks[i].toByteArray());
    }

    return ba;
}

/*!
 * Returns an unformatted textual version of the identity.
 *
 * See https://www.grc.com/sqrl/SQRL_Cryptography.pdf
 * on page 27 for further details.
 *
 * \note For a human-friendly version of the textual identity,
 * call \c getTextualVersionFormatted().
 *
 * \sa getTextualVersionFormatted
 */

QString IdentityModel::getTextualVersion()
{
    IdentityBlock* block2 = getBlock(2);
    IdentityBlock* block3 = getBlock(3);

    if (block2 == nullptr) return ""; // We need block 2

    QByteArray identityData = block2->toByteArray();
    if (block3 != nullptr) identityData += block3->toByteArray();

    return CryptUtil::base56EncodeIdentity(identityData);
}

/*!
 * Returns a formatted textual version of the identity.
 * The output looks something like this:
 *
 * jWJX JmD3 hUKQ qcRQ YRis
 * Gnmx uQHs LaE5 vR2f V7At
 * vW5g UQ5m B5kK gBjH 9uJr
 * JQEF T7sm yYm7 USzH tmfu
 * 6ktj U86K Dqdz p8Gh TfZS
 * UA8a G3F
 *
 * See https://www.grc.com/sqrl/SQRL_Cryptography.pdf
 * on page 27 for further details.
 *
 * \note For an unformatted version of the textual identity,
 * call \c getTextualVersion().
 *
 * \sa getTextualVersion
 */

QString IdentityModel::getTextualVersionFormatted()
{
    return CryptUtil::formatTextualIdentity(
                getTextualVersion());
}

/**************************************************************
 *************************************************************/

/*!
 *
 * \class IdentityBlock
 * \brief Represents a single identity block within the SQRL's
 * S4 identity storage format.
 *
 * \c IdentityBlock holds a vector of \c IdentityBlockItem objects,
 * representing a single block within the \c IdentityModel.
 *
 * More information SQRL's storage format can be found here:
 * https://www.grc.com/sqrl/SQRL_Cryptography.pdf
 *
 * \sa IdentityModel, IdentityBlockItem
 *
*/


/*!
 * Returns the first \c IdentityBlockItem which has its "name" property
 * set to \a name, or \c nullptr if such an item could not be found.
 */

IdentityBlockItem *IdentityBlock::getItem(QString name)
{
    for (auto iter=items.begin(); iter!=items.end(); iter++)
    {
        if (iter->name == name)
        {
            return &(*iter);
        }
    }

    return nullptr;
}

/*!
 * Duplicates \a item and inserts the exact copy into the block's
 * item list right behind \a item.
 *
 * Returns \c true if \a item was found and could be duplicated,
 * or \c false otherwise.
 */

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

/*!
 * Deletes \a item from the block's item list.
 *
 * Returns \c true if \a item was found and could be deleted,
 * or \c false otherwise.
 */

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

/*!
 * Moves \a item one step upwards or downwards within the block's
 * list of items (imagine a vertical index going from low index (up)
 * to high index (down)).
 *
 * If \a up is \c true, \a item will be moved upwards (index -= 1),
 * or downwards (index += 1) otherwise.
 */

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

/*!
 * Inserts \a item into the block's item list, placing it directly
 * after the existing item pointed to by \a after.
 *
 * Returns \c true if \a after was found and \a item was inserted,
 * and \c false if \a after could not be found in the list.
 */

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

/**************************************************************
 *************************************************************/

/*!
 *
 * \class IdentityBlockItem
 * \brief Represents a single item/setting within an \c IdentityBlock.
 *
 * \note Values are stored as strings within \c IdentityBlockItem
 * objects and only converted to their native types once needed.
 *
 * More information SQRL's storage format can be found at
 * https://www.grc.com/sqrl/SQRL_Cryptography.pdf
 *
 * \sa IdentityModel, IdentityBlock
 *
*/

std::map<ItemDataType, ItemDataTypeInfo> IdentityBlockItem::DataTypeMap = {
    {ItemDataType::UINT_8, {"UINT_8", 1}},
    {ItemDataType::UINT_16, {"UINT_16", 2}},
    {ItemDataType::UINT_32, {"UINT_32", 4}},
    {ItemDataType::BYTE_ARRAY, {"BYTE_ARRAY", 0}},
    {ItemDataType::STRING, {"STRING", 0}},
    {ItemDataType::UNDEFINED, {"UNDEFINED", 0}}
};

/*!
 * Searches the \c DataTypeMap for an \c ItemDataType with a name of
 * \a dataType. If the \c ItemDataType is found, it is being returned.
 * If no such data type exists, \c ItemDataType::UNDEFINED is returned.
 */

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

/*!
 * Returns a list of possible \c IdentityBlockItem data types.
 */

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

/*!
 * Returns the raw binary representation of the \c IdentityBlockItem.
 */

QByteArray IdentityBlockItem::toByteArray()
{
    QByteArray ba;

    if (this->dataType == UINT_8)
    {
        ba.append(static_cast<char>(this->value.toInt()));
    }
    else if (this->dataType == UINT_16)
    {
        int vi = this->value.toInt();
        char* data = static_cast<char*>(static_cast<void*>(&vi));
        ba.append(data[0]);
        ba.append(data[1]);
    }
    else if (this->dataType == UINT_32)
    {
        uint32_t value32 = static_cast<uint32_t>(this->value.toInt());
        char* data = static_cast<char*>(static_cast<void*>(&value32));
        ba.append(data[0]);
        ba.append(data[1]);
        ba.append(data[2]);
        ba.append(data[3]);
    }
    else if (this->dataType == BYTE_ARRAY)
    {
        QByteArray baTemp = QByteArray::fromHex(this->value.toLocal8Bit());
        ba.append(baTemp);
    }

    return ba;
}

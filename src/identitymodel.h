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

#ifndef IDENTITYMODEL_H
#define IDENTITYMODEL_H

#include "common.h"

struct ItemDataTypeInfo
{
    QString name;
    int nrOfBytes;
};

enum ItemDataType
{
    UINT_8,
    UINT_16,
    UINT_32,
    BYTE_ARRAY,
    STRING,
    UNDEFINED
};

// Forward declarations
class IdentityBlock;
class IdentityBlockItem;
class CryptUtil;

/**********************************************
 *    class IdentityModel                     *
 *********************************************/

class IdentityModel
{     
public:
    QList<IdentityBlock> blocks;

public:
    void writeToFile(QString fileName);
    IdentityBlock* getBlock(int blockType);
    QList<int> getAvailableBlockTypes();
    bool deleteBlock(IdentityBlock* block);
    bool moveBlock(IdentityBlock* block, bool up);
    bool insertBlock(IdentityBlock block, IdentityBlock* after);
    void clear();
    void import(IdentityModel& model);
    QByteArray getRawBytes();
    QString getTextualVersion();
    QString getTextualVersionFormatted();
    bool hasBlocks();
    bool hasBlockType(int blockType);
};

/**********************************************
 *    class IdentityBlock                     *
 *********************************************/

class IdentityBlock
{
public:
    int blockType = -1;
    QString description = "";
    QString color = "rgb(214, 201, 163)";
    QList<IdentityBlockItem> items;

public:
    IdentityBlockItem* getItem(QString name);
    bool duplicateItem(IdentityBlockItem* item);
    bool deleteItem(IdentityBlockItem* item);
    bool moveItem(IdentityBlockItem* item, bool up);
    bool insertItem(IdentityBlockItem item, IdentityBlockItem* after);
    QByteArray toByteArray();
};

/**********************************************
 *    class IdentityBlockItem                 *
 *********************************************/

class IdentityBlockItem
{
public:
    static QMap<ItemDataType, ItemDataTypeInfo> DataTypeMap;
    static ItemDataType findDataType(QString dataType);
    static QStringList getDataTypeList();
    QByteArray toByteArray();

public:
    QString name = "";
    QString description = "";
    ItemDataType dataType = ItemDataType::UNDEFINED;
    int nrOfBytes = 0;
    QString value = "";
    int repeatIndex = -1;
    int repeatCount = 1;
};

#endif // IDENTITYMODEL_H

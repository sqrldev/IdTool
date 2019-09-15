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

#include <vector>
#include <map>
#include <string>
#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QDebug>

// Forward declarations
class IdentityBlock;
class IdentityBlockItem;

class IdentityModel
{     
public:
    std::vector<IdentityBlock> blocks;

public:
    void writeToFile(QString fileName);
    void deleteBlock(IdentityBlock* block);
    bool moveBlock(IdentityBlock* block, bool up);
    bool insertBlock(IdentityBlock item, IdentityBlock* after);
    void clear();
};

class IdentityBlock
{
public:
    int blockType = -1;
    QString description = "";
    QString color = "rgb(214, 201, 163)";
    std::vector<IdentityBlockItem> items;

public:
    QByteArray toByteArray();
};

class IdentityBlockItem
{
public:
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
    QString name = "";
    QString description = "";
    QString type = "UNDEFINED";
    int bytes = 0;
    QString value = "";
    int repeatIndex = -1;
    int repeatCount = 1;
};

#endif // IDENTITYMODEL_H

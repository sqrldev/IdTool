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

#ifndef S4PARSER_H
#define S4PARSER_H

#include "identitymodel.h"
#include <fstream>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class IdentityParser
{
public:
    static const QString HEADER;
    static const QString HEADER_BASE64;

private:
    bool m_bIsBase64 = false;

public:
    void parseFile(QString fileName, IdentityModel* model);
    void parseText(QByteArray identityText, IdentityModel* model);
    static QByteArray getBlockDefinition(uint16_t blockType);
    static IdentityBlock createEmptyBlock(uint16_t blockType);

private:
    void parse(QByteArray data, IdentityModel* model);
    IdentityBlock parseBlock(const char* data, QJsonDocument* blockDef);
    bool checkHeader(QByteArray data);
    QByteArray getUnknownBlockDefinition();
    uint16_t getBlockLength(const char* data);
    uint16_t getBlockType(const char* data);
    QString parseUint8(const char* data, int offset);
    QString parseUint16(const char* data, int offset);
    QString parseUint32(const char* data, int offset);
    QString parseByteArray(const char* data, int offset, int bytes);
};

#endif // S4PARSER_H

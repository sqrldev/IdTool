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

private:
    void parse(QByteArray data, IdentityModel* model);
    IdentityModel::IdentityBlock parseBlock(const char* data, QJsonDocument* blockDef);
    bool checkHeader(QByteArray data);
    QByteArray getBlockDefinition(uint16_t blockType);
    uint16_t getBlockLength(const char* data);
    uint16_t getBlockType(const char* data);
    QString parseUint8(const char* data, int offset);
    QString parseUint16(const char* data, int offset);
    QString parseUint32(const char* data, int offset);
    QString parseByteArray(const char* data, int offset, int bytes);
};

#endif // S4PARSER_H

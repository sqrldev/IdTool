#ifndef CRYPTUTIL_H
#define CRYPTUTIL_H

#include <QString>
#include <QByteArray>
#include "sodium.h"

class CryptUtil
{
public:
    CryptUtil();
    static QByteArray enSCryptIterations(QString password, QByteArray randomSalt, int logNFactor, int iterationCount);
    static QByteArray xorByteArrays(QByteArray a, QByteArray b);
};

#endif // CRYPTUTIL_H

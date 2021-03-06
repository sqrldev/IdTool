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

#ifndef CRYPTUTIL_H
#define CRYPTUTIL_H

#include "common.h"
#include "identitymodel.h"
#include "identityparser.h"
#include "sodium.h"

/**********************************************
 *    class CryptUtil                         *
 *********************************************/

class CryptUtil
{
public:
    static const QByteArray BASE56_ALPHABET;
    static const int BASE56_LINE_MAX_CHARS;
    static const int BASE56_BASE_NUM;

public:
    CryptUtil();
    static QByteArray xorByteArrays(QByteArray a, QByteArray b);
    static bool getRandomBytes(QByteArray& buffer);
    static bool getRandomByte(unsigned char& byte);
    static bool enScryptIterations(QByteArray& result, QString password, QByteArray randomSalt, int logNFactor, int iterationCount, QProgressDialog* progressDialog = nullptr);
    static bool enScryptTime(QByteArray& result, int& iterationCount, QString password, QByteArray randomSalt, int logNFactor, int secondsToRun, QProgressDialog* progressDialog = nullptr);
    static bool decryptBlock1(QByteArray& decryptedImk, QByteArray& decryptedIlk, IdentityBlock *block, QByteArray key);
    static bool decryptBlock2(QByteArray& decryptedIuk, IdentityBlock *block, QString rescueCode, QProgressDialog* progressDialog = nullptr);
    static bool decryptBlock3(QList<QByteArray>& decryptedPreviousIuks, IdentityBlock *block, QByteArray imk);
    static bool createSiteKeys(QByteArray& publicKey, QByteArray& privateKey, QString domain, QString altId, QByteArray imk);
    static bool createKeyFromPassword(QByteArray& key, IdentityBlock& block, QString password, QProgressDialog* progressDialog = nullptr);
    static QString getHostLowercase(QString url);
    static QString makeHostLowercase(QString url);
    static QByteArray createImkFromIuk(QByteArray decryptedIuk);
    static QByteArray createIlkFromIuk(QByteArray decryptedIuk);
    static QByteArray createIndexedSecret(QByteArray imk, QString domain, QString altId, QByteArray secretIndex);
    static QByteArray enHash(QByteArray data);
    static IdentityBlock createBlock1(QByteArray iuk, QString password, QProgressDialog* progressDialog = nullptr);
    static IdentityBlock createBlock2(QByteArray iuk, QString rescueCode, QProgressDialog* progressDialog = nullptr);
    static bool updateBlock1WithPassword(IdentityBlock* oldBlock, IdentityBlock* updatedBlock, QString password, QString newPassword, QProgressDialog* progressDialog = nullptr);
    static bool updateBlock1(IdentityBlock* block1, QByteArray unencryptedImk, QByteArray unencryptedIlk, QString newPassword, QProgressDialog* progressDialog = nullptr);
    static bool updateBlock2(IdentityBlock* block2, QByteArray unencryptedIuk, QString rescueCode, int secondsToRunScrypt = -1, QProgressDialog* progressDialog = nullptr);
    static QByteArray aesGcmEncrypt(QByteArray message, QByteArray additionalData, QByteArray iv, QByteArray key);
    static QByteArray createIuk();
    static QString createNewRescueCode();
    static QString formatRescueCode(QString rescueCode);
    static bool createIdentity(IdentityModel& identity, QString &rescueCode, QString password, QProgressDialog *progressDialog = nullptr);
    static QByteArray reverseByteArray(QByteArray source);
    static BigUnsigned convertByteArrayToBigUnsigned(QByteArray data);
    static QByteArray convertBigUnsignedToByteArray(BigUnsigned bigNum);
    static QString base56EncodeIdentity(QByteArray identityData);
    static char createBase56CheckSumChar(QByteArray dataBytes);
    static QByteArray base56DecodeIdentity(QString textualIdentity);
    static QString formatTextualIdentity(QString textualIdentity, bool escapeNewline = false);
    static bool verifyTextualIdentity(QString textualIdentity);
    static QString stripWhitespace(QString source);
};

#endif // CRYPTUTIL_H

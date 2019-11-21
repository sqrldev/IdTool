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
#include "sodium.h"

class CryptUtil
{
public:
    CryptUtil();
    static QByteArray xorByteArrays(QByteArray a, QByteArray b);
    static bool enSCryptIterations(QByteArray& result, QString password, QByteArray randomSalt, int logNFactor, int iterationCount, QProgressDialog* progressDialog = nullptr);
    static bool decryptBlock1(QByteArray& decryptedImk, QByteArray& decryptedIlk, IdentityBlock *block, QByteArray key);
    static bool decryptBlock2(QByteArray& decryptedIuk, IdentityBlock *block, QString rescueCode, QProgressDialog* progressDialog = nullptr);
    static bool decryptBlock3(QList<QByteArray>& decryptedPreviousIuks, IdentityBlock *block, QByteArray imk);
    static bool createSiteKeys(QByteArray& publicKey, QByteArray& privateKey, QString domain, QByteArray imk);
    static QByteArray createKeyFromPassword(IdentityBlock* block, QString password, QProgressDialog* progressDialog = nullptr);
    static QByteArray createImkFromIuk(QByteArray decryptedIuk);
    static QByteArray enHash(QByteArray data);
    static bool updateBlock1(IdentityBlock *oldBlock, IdentityBlock* updatedBlock, QByteArray key);
    static QByteArray CreateIuk();
    static QString CreateNewRescueCode();
    static QString FormatRescueCode(QString rescueCode);
};

#endif // CRYPTUTIL_H

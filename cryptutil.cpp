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

#include "cryptutil.h"

CryptUtil::CryptUtil()
{

}

QByteArray CryptUtil::enSCryptIterations(QString password, QByteArray randomSalt, int logNFactor, int iterationCount)
{
    const int KEYLENGTH = 32;
    QByteArray pwdBytes = password.toLocal8Bit();
    QByteArray key(KEYLENGTH, 0);

    int ret = crypto_pwhash_scryptsalsa208sha256_ll(
                reinterpret_cast<const unsigned char*>(pwdBytes.constData()),
                static_cast<size_t>(pwdBytes.length()),
                reinterpret_cast<const unsigned char*>(randomSalt.constData()),
                static_cast<size_t>(randomSalt.length()),
                1 << logNFactor,
                256,
                1,
                reinterpret_cast<uint8_t*>(key.data()),
                static_cast<size_t>(key.length()));

    QByteArray xorKey(key);

    for(int i = 1; i < iterationCount; i++)
    {
        ret = crypto_pwhash_scryptsalsa208sha256_ll(
                    reinterpret_cast<const unsigned char*>(pwdBytes.constData()),
                    static_cast<size_t>(pwdBytes.length()),
                    reinterpret_cast<const unsigned char*>(key.constData()),
                    static_cast<size_t>(key.length()),
                    1 << logNFactor,
                    256,
                    1,
                    reinterpret_cast<uint8_t*>(key.data()),
                    static_cast<size_t>(key.length()));

        xorKey = xorByteArrays(key, xorKey);
    }

    return xorKey;
}

QByteArray CryptUtil::xorByteArrays(QByteArray a, QByteArray b)
{
    QByteArray result;

    for (int i = 0; i < a.length(); i++)
    {
        result[i] = a[i] ^ b[i % (b.length()-1)];
    }
    return result;
}

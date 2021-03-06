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

#include "testcryptutil.h"
#include "../testutils.h"
#include "../../src/cryptutil.h"


void TestCryptUtil::reverseByteArray()
{
    QCOMPARE(CryptUtil::reverseByteArray(QString("abcdefg").toLocal8Bit()),
             QString("gfedcba").toLocal8Bit());

    QCOMPARE(CryptUtil::reverseByteArray(QString("a").toLocal8Bit()),
             QString("a").toLocal8Bit());

    QCOMPARE(CryptUtil::reverseByteArray(QByteArray("")),
             QByteArray(""));

    QCOMPARE(CryptUtil::reverseByteArray(QByteArray::fromHex("bf5211ff72")),
             QByteArray::fromHex("72ff1152bf"));
}

void TestCryptUtil::createSiteKeys()
{
    QList<QList<QByteArray>> vectors = TestUtils::parseVectorsCsv("vectors/identity-vectors.txt");
    if (vectors.count() < 1) QFAIL("No vectors found!");

    for (QList<QByteArray> vector : vectors)
    {
        if (vector.isEmpty()) continue;

        QByteArray imk = QByteArray::fromBase64(vector.at(2), QByteArray::Base64UrlEncoding);
        QByteArray domain = vector.at(3);
        QByteArray altId = vector.at(4);
        QByteArray expectedResult = QByteArray::fromBase64(vector.at(5), QByteArray::Base64UrlEncoding);

        QByteArray pubKey(crypto_sign_PUBLICKEYBYTES, 0);
        QByteArray privKey(crypto_sign_SECRETKEYBYTES, 0);
        CryptUtil::createSiteKeys(pubKey, privKey, domain, altId, imk);

        QCOMPARE(expectedResult, pubKey);
    }
}

void TestCryptUtil::createIndexedSecret()
{
    QList<QList<QByteArray>> vectors = TestUtils::parseVectorsCsv("vectors/ins-vectors.txt");
    if (vectors.count() < 1) QFAIL("No vectors found!");

    for (QList<QByteArray> vector : vectors)
    {
        if (vector.isEmpty()) continue;

        QByteArray imk = QByteArray::fromBase64(vector.at(0), QByteArray::Base64UrlEncoding);
        QByteArray domain = vector.at(1);
        QByteArray altId = "";
        QByteArray secretIndex = vector.at(2);
        QByteArray expectedIns = QByteArray::fromBase64(vector.at(3), QByteArray::Base64UrlEncoding);

        QByteArray ins = CryptUtil::createIndexedSecret(imk, domain, altId, secretIndex);

        QCOMPARE(ins, expectedIns);
    }
}

void TestCryptUtil::enScryptIterations()
{
    QSKIP("Test takes too long for regular runs. Skipping...");
    QList<QList<QByteArray>> vectors = TestUtils::parseVectorsCsv("vectors/enscrypt-vectors.txt");
    if (vectors.count() < 1) QFAIL("No vectors found!");

    for (QList<QByteArray> vector : vectors)
    {
        QByteArray expectedResult = QByteArray::fromHex(vector.at(4));
        QByteArray randomSalt = vector.at(1);
        QString password = QString::fromLocal8Bit(vector.at(0).data());
        int iterationCount = vector.at(2).toInt();
        int logNFactor = 9;

        QByteArray result(32, 0);

        CryptUtil::enScryptIterations(result,
                           password,
                           randomSalt,
                           logNFactor,
                           iterationCount);

       QCOMPARE(result, expectedResult);
    }
}

void TestCryptUtil::getHostLowercase()
{
    QCOMPARE(CryptUtil::getHostLowercase("www.Example.com"), "www.example.com");
    QCOMPARE(CryptUtil::getHostLowercase("www.example.com"), "www.example.com");
    QCOMPARE(CryptUtil::getHostLowercase("www.Example.com/TEST"), "www.example.com");
    QCOMPARE(CryptUtil::getHostLowercase("https://www.Example.com"), "www.example.com");
    QCOMPARE(CryptUtil::getHostLowercase("sqrl://test:test@www.Example.com"), "www.example.com");
    QCOMPARE(CryptUtil::getHostLowercase("Example.com"), "example.com");
    QCOMPARE(CryptUtil::getHostLowercase("Example.com/pATH"), "example.com");
    QCOMPARE(CryptUtil::getHostLowercase("sqrl.EXAMPLE.com"), "sqrl.example.com");
    QCOMPARE(CryptUtil::getHostLowercase("SQRL.Example.com"), "sqrl.example.com");
}

void TestCryptUtil::makeHostLowercase()
{
    QCOMPARE(CryptUtil::makeHostLowercase("www.Example.com"), "www.example.com");
    QCOMPARE(CryptUtil::makeHostLowercase("Www.Example.com"), "www.example.com");
    QCOMPARE(CryptUtil::makeHostLowercase("www.example.com"), "www.example.com");
    QCOMPARE(CryptUtil::makeHostLowercase("www.Example.com/TEST"), "www.example.com/TEST");
    QCOMPARE(CryptUtil::makeHostLowercase("https://www.Example.com"), "https://www.example.com");
    QCOMPARE(CryptUtil::makeHostLowercase("SQRL://teSt:Test@www.Example.coM"), "SQRL://teSt:Test@www.example.com");
    QCOMPARE(CryptUtil::makeHostLowercase("Example.com"), "example.com");
    QCOMPARE(CryptUtil::makeHostLowercase("sqrl.EXAMPLE.com"), "sqrl.example.com");
    QCOMPARE(CryptUtil::makeHostLowercase("SQRL.Example.com/cAsE"), "sqrl.example.com/cAsE");
}

void TestCryptUtil::enHash()
{
    QList<QList<QByteArray>> vectors = TestUtils::parseVectorsCsv("vectors/enhash-vectors.txt");
    if (vectors.count() < 1) QFAIL("No vectors found!");

    for (QList<QByteArray> vector : vectors)
    {
        QByteArray input = QByteArray::fromBase64(vector.at(0), QByteArray::Base64UrlEncoding);
        QByteArray expectedResult = QByteArray::fromBase64(vector.at(1), QByteArray::Base64UrlEncoding);

        QByteArray result = CryptUtil::enHash(input);
        QCOMPARE(result, expectedResult);
    }
}

void TestCryptUtil::base56Encode()
{
    QList<QList<QByteArray>> vectors = TestUtils::parseVectorsCsv("vectors/base56-vectors.txt");
    if (vectors.count() < 1) QFAIL("No vectors found!");

    for (QList<QByteArray> vector : vectors)
    {
        QByteArray input = QByteArray::fromHex(vector.at(0));
        char lineNr = static_cast<char>(vector.at(1).toInt());
        QByteArray expectedResult = vector.at(2);
        char expectedCheckChar = vector.at(3).at(0);

        QString result = CryptUtil::base56EncodeIdentity(input);
        // Remove check character for this test, since
        // we will calcualte it separately
        result.remove(result.length()-1, 1);

        QCOMPARE(result, QString::fromLocal8Bit(expectedResult));

        result.append(lineNr);

        char checkChar = CryptUtil::createBase56CheckSumChar(result.toLocal8Bit());
        QCOMPARE(checkChar, expectedCheckChar);
    }
}

void TestCryptUtil::base56EncodeDecodeFullFormat()
{
    QList<QList<QByteArray>> vectors = TestUtils::parseVectorsCsv(
                "vectors/base56-full-format-vectors.txt");
    if (vectors.count() < 1) QFAIL("No vectors found!");

    for (QList<QByteArray> vector : vectors)
    {
        QByteArray input = QByteArray::fromBase64(vector.at(0), QByteArray::Base64UrlEncoding);
        //QByteArray input = QByteArray::fromHex(vector.at(1));
        QString expectedResult = vector.at(2);

        //Remove spaces and newlines encoded as "\n"
        expectedResult = expectedResult.replace("\\n", "").replace(" ", "");

        QString result = CryptUtil::base56EncodeIdentity(input);
        QCOMPARE(result, expectedResult);

        QByteArray decodeResult = CryptUtil::base56DecodeIdentity(result);
        QCOMPARE(decodeResult, input);
    }
}

void TestCryptUtil::base56EncodeDecodeRandomInput()
{
    for (int i=0; i<128; i++)
    {
        QByteArray input(i, 0);
        CryptUtil::getRandomBytes(input);

        QString base56 = CryptUtil::base56EncodeIdentity(input);
        if (!CryptUtil::verifyTextualIdentity(base56))
            qDebug() << "Base56 verification failed! Length:" << base56.length() << " Base56 string: " << base56;
        QByteArray result = CryptUtil::base56DecodeIdentity(base56);

        QCOMPARE(result, input);
    }
}

void TestCryptUtil::identityKeys()
{
    QList<QList<QByteArray>> vectors = TestUtils::parseVectorsCsv("vectors/identity-vectors.txt");
    if (vectors.count() < 1) QFAIL("No vectors found!");

    for (QList<QByteArray> vector : vectors)
    {
        if (vector.isEmpty()) continue;

        QByteArray iuk = QByteArray::fromBase64(vector.at(0), QByteArray::Base64UrlEncoding);
        QByteArray ilk = QByteArray::fromBase64(vector.at(1), QByteArray::Base64UrlEncoding);
        QByteArray imk = QByteArray::fromBase64(vector.at(2), QByteArray::Base64UrlEncoding);
        QByteArray domain = vector.at(3);
        QByteArray altId = vector.at(4);
        QByteArray idk = QByteArray::fromBase64(vector.at(5), QByteArray::Base64UrlEncoding);

        QByteArray computedIlk = CryptUtil::createIlkFromIuk(iuk);
        QCOMPARE(computedIlk, ilk);

        QByteArray computedImk = CryptUtil::createImkFromIuk(iuk);
        QCOMPARE(computedImk, imk);

        QByteArray pubKey(crypto_sign_PUBLICKEYBYTES, 0);
        QByteArray privKey(crypto_sign_SECRETKEYBYTES, 0);
        CryptUtil::createSiteKeys(pubKey, privKey, domain, altId, imk);

        QCOMPARE(idk, pubKey);
    }
}

QTEST_MAIN(TestCryptUtil)

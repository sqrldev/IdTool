#include "testcryptutil.h"
#include "../../cryptutil.h"

QList<QList<QByteArray>> TestCryptUtil::parseVectorsCsv(QString fileName,
                            bool skipFirstLine, bool removeQuotes)
{
    QFile input(fileName);
    input.open(QIODevice::ReadOnly);
    QByteArray data = input.readAll();
    input.close();

    QList<QList<QByteArray>> result;
    QList<QByteArray> lines =  data.split('\n');

    for (int i=0; i<lines.count(); i++)
    {
        if (i==0 && skipFirstLine) continue;
        if (lines.at(i).isEmpty()) continue;


        QList<QByteArray> lineResult;
        QList<QByteArray> fields = lines.at(i).split(',');

        for (QByteArray field : fields)
        {
            if (field.length() > 1)
            {
                // Since we split by "\n", there might be some
                // orphaned "\r" at the end of the last field,
                // which we need to get rid of
                if (field[field.length()-1] == '\r')
                {
                    field = field.left(field.length()-1);
                }

                if (removeQuotes)
                {
                    if (field[0] == '"') field.remove(0, 1);
                    if (field[field.length()-1] == '"') field.remove(field.length()-1, 1);
                }
            }

            lineResult.append(field);
        }

        result.append(lineResult);
    }

    return result;
}

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
    QList<QList<QByteArray>> vectors = parseVectorsCsv("vectors/identity-vectors.txt");

    for (QList<QByteArray> vector : vectors)
    {
        if (vector.isEmpty()) continue;

        QByteArray imk = QByteArray::fromBase64(vector.at(0), QByteArray::Base64UrlEncoding);
        QByteArray domain = vector.at(1);
        QByteArray altId = vector.at(2);
        QByteArray expectedResult = QByteArray::fromBase64(vector.at(3), QByteArray::Base64UrlEncoding);

        QByteArray pubKey(crypto_sign_PUBLICKEYBYTES, 0);
        QByteArray privKey(crypto_sign_SECRETKEYBYTES, 0);
        CryptUtil::createSiteKeys(pubKey, privKey, domain, altId, imk);

        QCOMPARE(expectedResult, pubKey);
    }
}

void TestCryptUtil::enScryptIterations()
{
    QSKIP("Test takes too long for regular runs. Skipping...");
    QList<QList<QByteArray>> vectors = parseVectorsCsv("vectors/enscrypt-vectors.txt");

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

QTEST_MAIN(TestCryptUtil)

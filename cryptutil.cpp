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

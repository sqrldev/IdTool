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

QByteArray CryptUtil::xorByteArrays(QByteArray a, QByteArray b)
{
    QByteArray result;

    for (int i = 0; i < a.length(); i++)
    {
        result[i] = a[i] ^ b[i % b.length()];
    }
    return result;
}

bool CryptUtil::enSCryptIterations(QByteArray& result, QString password, QByteArray randomSalt,
                        int logNFactor, int iterationCount, QProgressDialog* progressDialog)
{
    const int KEYLENGTH = 32;
    QByteArray pwdBytes = password.toLocal8Bit();
    QByteArray key(KEYLENGTH, 0);

    if (sodium_init() < 0) return false;

    if (progressDialog != nullptr) progressDialog->setMaximum(iterationCount);

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

    if (progressDialog != nullptr) progressDialog->setValue(1);

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

        if (progressDialog != nullptr)
        {
            progressDialog->setValue(i);
            if (progressDialog->wasCanceled()) return false;
        }

        xorKey = xorByteArrays(key, xorKey);
    }

    result = xorKey;
    return true;
}

bool CryptUtil::decryptBlock1(QByteArray& decryptedImk, QByteArray& decryptedIlk,
                IdentityBlock *block, QByteArray key)
{
    QByteArray decryptedIdentityKeys(64, 0);

    if (decryptedImk == nullptr || decryptedIlk == nullptr || block == nullptr ||
            sodium_init() < 0 || crypto_aead_aes256gcm_is_available() == 0)
    {
        return false;
    }

    QByteArray aesGcmIV = QByteArray::fromHex(block->items[3].value.toLocal8Bit());
    QByteArray encryptedImk = QByteArray::fromHex(block->items[11].value.toLocal8Bit());
    QByteArray encryptedIlk = QByteArray::fromHex(block->items[12].value.toLocal8Bit());
    QByteArray verificationTag = QByteArray::fromHex(block->items[13].value.toLocal8Bit());
    QByteArray encryptedIdentityKeys;
    encryptedIdentityKeys.append(encryptedImk);
    encryptedIdentityKeys.append(encryptedIlk);
    QByteArray plainText;
    for (size_t i=0; i<11; i++) plainText.append(block->items[i].toByteArray());

    int ret = crypto_aead_aes256gcm_decrypt_detached(
                reinterpret_cast<unsigned char*>(decryptedIdentityKeys.data()),
                nullptr,
                reinterpret_cast<const unsigned char*>(encryptedIdentityKeys.constData()),
                static_cast<unsigned long long>(encryptedIdentityKeys.length()),
                reinterpret_cast<const unsigned char*>(verificationTag.constData()),
                reinterpret_cast<const unsigned char*>(plainText.constData()),
                static_cast<unsigned long long>(plainText.length()),
                reinterpret_cast<const unsigned char*>(aesGcmIV.constData()),
                reinterpret_cast<const unsigned char*>(key.constData()));

    if (ret != 0) return false;

    decryptedImk = decryptedIdentityKeys.left(32);
    decryptedIlk = decryptedIdentityKeys.mid(32);

    return true;
}

bool CryptUtil::decryptBlock2(QByteArray &decryptedIuk, IdentityBlock *block, QString rescueCode, QProgressDialog *progressDialog)
{
    if (decryptedIuk == nullptr || block == nullptr ||
            sodium_init() < 0 || crypto_aead_aes256gcm_is_available() == 0)
    {
        return false;
    }

    QByteArray aesGcmIV(12, 0);
    QByteArray scryptSalt = QByteArray::fromHex(block->items[2].value.toLocal8Bit());
    int scryptLogNFactor = block->items[3].value.toInt();
    int scryptIterationCount = block->items[4].value.toInt();
    QByteArray encryptedIuk = QByteArray::fromHex(block->items[5].value.toLocal8Bit());
    QByteArray verificationTag = QByteArray::fromHex(block->items[6].value.toLocal8Bit());
    QByteArray plainText;
    for (size_t i=0; i<5; i++) plainText.append(block->items[i].toByteArray());

    QByteArray key;
    bool ok = CryptUtil::enSCryptIterations(
                key,
                rescueCode,
                scryptSalt,
                scryptLogNFactor,
                scryptIterationCount,
                progressDialog);

    if (!ok) return false;

    int ret = crypto_aead_aes256gcm_decrypt_detached(
                reinterpret_cast<unsigned char*>(decryptedIuk.data()),
                nullptr,
                reinterpret_cast<const unsigned char*>(encryptedIuk.constData()),
                static_cast<unsigned long long>(encryptedIuk.length()),
                reinterpret_cast<const unsigned char*>(verificationTag.constData()),
                reinterpret_cast<const unsigned char*>(plainText.constData()),
                static_cast<unsigned long long>(plainText.length()),
                reinterpret_cast<const unsigned char*>(aesGcmIV.constData()),
                reinterpret_cast<const unsigned char*>(key.constData()));

    if (ret != 0) return false;

    return true;
}

bool CryptUtil::decryptBlock3(QList<QByteArray> &decryptedPreviousIuks, IdentityBlock *block, QByteArray imk)
{
    bool ok = false;

    if (block == nullptr || sodium_init() < 0 ||
            crypto_aead_aes256gcm_is_available() == 0)
    {
        return false;
    }

    QByteArray aesGcmIV(12, 0);
    int nrOfPreviousIuks = block->items[2].value.toInt(&ok);
    if (!ok) return false;

    QByteArray verificationTag = QByteArray::fromHex(
                block->items[3 + static_cast<size_t>(nrOfPreviousIuks)].value.toLocal8Bit()
            );

    QByteArray plainText;
    for (size_t i=0; i<3; i++) plainText.append(block->items[i].toByteArray());

    QByteArray encryptedData;

    for (size_t i=0; i<static_cast<size_t>(nrOfPreviousIuks); i++)
    {
        encryptedData.append(
                    QByteArray::fromHex(block->items[3+i].value.toLocal8Bit())
                );
    }

    QByteArray decryptedData(encryptedData.size(), 0);

    int ret = crypto_aead_aes256gcm_decrypt_detached(
                reinterpret_cast<unsigned char*>(decryptedData.data()),
                nullptr,
                reinterpret_cast<const unsigned char*>(encryptedData.constData()),
                static_cast<unsigned long long>(encryptedData.length()),
                reinterpret_cast<const unsigned char*>(verificationTag.constData()),
                reinterpret_cast<const unsigned char*>(plainText.constData()),
                static_cast<unsigned long long>(plainText.length()),
                reinterpret_cast<const unsigned char*>(aesGcmIV.constData()),
                reinterpret_cast<const unsigned char*>(imk.constData()));

    if (ret != 0) return false;

    for (int i=0; i<nrOfPreviousIuks; i++)
    {
        decryptedPreviousIuks.append(
                    decryptedData.mid(i*32, 32)
                );
    }

    return true;
}

bool CryptUtil::createSiteKeys(QByteArray& publicKey, QByteArray& privateKey, QString domain, QByteArray imk)
{
    QByteArray domainBytes = domain.toLocal8Bit();
    unsigned char seed[crypto_sign_SEEDBYTES];

    int ret = crypto_auth_hmacsha256(
                seed,
                reinterpret_cast<const unsigned char*>(domainBytes.constData()),
                static_cast<unsigned long long>(domainBytes.length()),
                reinterpret_cast<const unsigned char*>(imk.constData()));

    if (ret != 0) return false;

    ret = crypto_sign_seed_keypair(
                reinterpret_cast<unsigned char*>(publicKey.data()),
                reinterpret_cast<unsigned char*>(privateKey.data()),
                seed);

    if (ret != 0) return false;

    return true;
}

QByteArray CryptUtil::createKeyFromPassword(IdentityBlock* block, QString password, QProgressDialog* progressDialog)
{
    QByteArray scryptSalt = QByteArray::fromHex(block->items[4].value.toLocal8Bit());
    int scryptLogNFactor = block->items[5].value.toInt();
    int scryptIterationCount = block->items[6].value.toInt();

    QByteArray key;
    bool ok = CryptUtil::enSCryptIterations(
                key,
                password,
                scryptSalt,
                scryptLogNFactor,
                scryptIterationCount,
                progressDialog);

    if (!ok) throw std::runtime_error(QObject::tr("enScrypt failed!")
                                .toStdString());

    return key;
}

QByteArray CryptUtil::createImkFromIuk(QByteArray decryptedIuk)
{
    return enHash(decryptedIuk);
}

QByteArray CryptUtil::enHash(QByteArray data)
{
    QByteArray output(data);
    QByteArray result(32, 0);

    for (int i=0; i<16; i++)
    {
        crypto_hash_sha256(
                    reinterpret_cast<unsigned char*>(output.data()),
                    reinterpret_cast<const unsigned char*>(output.constData()),
                    static_cast<unsigned long long>(output.length())
                    );

        if (i == 0) result.replace(0, result.length(), output);
        else result = xorByteArrays(result, output);
    }

    return result;
}

bool CryptUtil::updateBlock1(IdentityBlock *oldBlock, IdentityBlock* updatedBlock, QByteArray key)
{
    QByteArray oldImk(32, 0);
    QByteArray oldIlk(32, 0);
    QByteArray newImk(32, 0);
    QByteArray newIlk(32, 0);
    QByteArray newIv(12, 0);
    QByteArray newPlainText;

    if (sodium_init() < 0) return false;

    bool ok = decryptBlock1(oldImk, oldIlk, oldBlock, key);
    if (!ok) return false;

    randombytes_buf(newIv.data(), static_cast<size_t>(newIv.length()));
    updatedBlock->items[3].value = newIv.toHex();

    QByteArray unencryptedKeys = oldImk + oldIlk;
    QByteArray encryptedKeys(unencryptedKeys.length(), 0);
    QByteArray authTag(16, 0);
    for (size_t i=0; i<11; i++) newPlainText.append(updatedBlock->items[i].toByteArray());
    unsigned long long authTagLen;

    crypto_aead_aes256gcm_encrypt_detached(
                reinterpret_cast<unsigned char*>(encryptedKeys.data()),
                reinterpret_cast<unsigned char*>(authTag.data()),
                &authTagLen,
                reinterpret_cast<const unsigned char*>(unencryptedKeys.constData()),
                static_cast<size_t>(unencryptedKeys.length()),
                reinterpret_cast<const unsigned char*>(newPlainText.constData()),
                static_cast<size_t>(newPlainText.length()),
                nullptr,
                reinterpret_cast<const unsigned char*>(newIv.constData()),
                reinterpret_cast<const unsigned char*>(key.constData()));

    newImk = encryptedKeys.left(32);
    newIlk = encryptedKeys.right(32);

    updatedBlock->items[11].value = newImk.toHex();
    updatedBlock->items[12].value = newIlk.toHex();
    updatedBlock->items[13].value = authTag.toHex();

    return true;
}

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

/*!
 *
 * \class CryptUtil
 *
 * \brief A collection of static utility functions mostly dealing with
 * SQRL-related cryptographic operations.
 *
 * \c CryptUtil implements most of the functionality described in
 * https://www.grc.com/sqrl/SQRL_Cryptography.pdf
 *
*/

const QByteArray CryptUtil::BASE56_ALPHABET = "23456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnpqrstuvwxyz";
const int CryptUtil::BASE56_LINE_MAX_CHARS = 19;
const int CryptUtil::BASE56_BASE_NUM = 56;

CryptUtil::CryptUtil() {}

/*!
 * Performs a byte-by-byte XOR-operation on the given byte arrays \a a and \a b
 * and returns the result. In order to maintain the cryptographic strengh of the
 * XOR operation, both byte arrays must be of the same length (number of bytes)
 * in order to avoid reuse of source material.
 *
 * \throws std::runtime_error is thrown if the byte arrays are not of equal size.
 */

QByteArray CryptUtil::xorByteArrays(QByteArray a, QByteArray b)
{
    QByteArray result(a.length(), 0);

    if (a.length() != b.length())
        throw std::runtime_error("Both byte arrays must be of the same size!");

    for (int i = 0; i < a.length(); i++)
    {
        result[i] = a[i] ^ b[i];
    }
    return result;
}

/*!
 * Fills \a buffer with \c buffer.length() number of random bytes.
 *
 * \return Retrurns true on success, false otherwise (e.g. if initializing
 * the crypto library failed)
 */

bool CryptUtil::getRandomBytes(QByteArray &buffer)
{
    if (sodium_init() < 0) return false;

    randombytes(reinterpret_cast<unsigned char*>(buffer.data()),
                static_cast<size_t>(buffer.size()));
    return true;
}

/*!
 * Assigns a single random byte to \a byte.
 *
 * \return Retrurns true on success, false otherwise (e.g. if initializing
 * the crypto library failed).
 */

bool CryptUtil::getRandomByte(unsigned char &byte)
{
    if (sodium_init() < 0) return false;

    QByteArray ba(1, 0);
    if (!getRandomBytes(ba)) return false;

    byte = static_cast<unsigned char>(ba[0]);
    return true;
}

/*!
 * Runs \a iterationCount number of scrypt iterations on \a password,
 * using the scrypt parameters \a randomSalt and \a logNFactor.
 *
 * If a valid \a progressDialog pointer is given, the operation will use it
 * to publish its progress. Otherwise, it will be ignored.
 *
 * When successful, the result of the operation will be stored in \a result.
 *
 * \return True on success, false otherwise (e.g. if initializing
 * the crypto library failed).
 */

bool CryptUtil::enScryptIterations(QByteArray& result, QString password, QByteArray randomSalt,
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

/*!
 * Runs multiple scrypt iterations on \a password for a duration of
 * \a secondsToRun seconds, using the the scrypt parameters \a randomSalt
 * and \a logNFactor.
 *
 * If a valid \a progressDialog pointer is given, the operation will use it
 * to publish its progress. Otherwise, it will be ignored.
 *
 * When successful, the result of the operation will be stored in \a result
 * and the number of iterations that were run is placed in \a iterationCount.
 *
 * \return True on success, false otherwise (e.g. if initializing
 * the crypto library failed).
 */

bool CryptUtil::enScryptTime(QByteArray &result, int &iterationCount, QString password,
                             QByteArray randomSalt, int logNFactor, int secondsToRun,
                             QProgressDialog *progressDialog)
{
    const int KEYLENGTH = 32;
    QByteArray pwdBytes = password.toLocal8Bit();
    QByteArray key(KEYLENGTH, 0);

    if (sodium_init() < 0) return false;

    if (progressDialog != nullptr) progressDialog->setMaximum(secondsToRun*1000);

    QElapsedTimer timer;
    timer.start();

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

    if (progressDialog != nullptr) progressDialog->setValue(static_cast<int>(timer.elapsed()));
    iterationCount = 1;

    QByteArray xorKey(key);

    for(int i = 1; ; i++)
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
            progressDialog->setValue(static_cast<int>(timer.elapsed()));
            if (progressDialog->wasCanceled()) return false;
        }

        xorKey = xorByteArrays(key, xorKey);

        iterationCount++;
        if (timer.elapsed() >= secondsToRun*1000) break;
    }

    result = xorKey;
    return true;
}

/*!
 * Decrypts the IMK and ILK contained within \a block using \a key, and
 * upon success places them into \a decryptedImk and \a decryptedIlk.
 *
 * \return True on success, false otherwise (e.g. if initializing
 * the crypto library failed).
 */

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
    QByteArray encryptedIdentityKeys = encryptedImk + encryptedIlk;
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

/*!
 * Decrypts the IUK contained within \a block using \a rescueCode, and
 * upon success places it into \a decryptedIuk.
 *
 * If a valid \a progressDialog pointer is given, the operation will use it
 * to publish its progress. Otherwise, it will be ignored.
 *
 * \return True on success, false otherwise (e.g. if initializing
 * the crypto library failed).
 */

bool CryptUtil::decryptBlock2(QByteArray &decryptedIuk, IdentityBlock *block, QString rescueCode,
                              QProgressDialog *progressDialog)
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
    bool ok = CryptUtil::enScryptIterations(
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

/*!
 * Decrypts a list of previous IUKs contained within \a block using \a imk as the key,
 * and upon success places them into \a decryptedPreviousIuks.
 *
 * \return True on success, false otherwise (e.g. if initializing
 * the crypto library failed).
 */

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

/*!
 * Creates a public- and private-key pair for \a domain using the identity master key \a imk.
 * Upon success, the public and private keys are stored within \a publicKey and \a privateKey.
 *
 * \return True on success, false otherwise (e.g. if initializing
 * the crypto library failed).
 */

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

/*!
 * Derives a key from the given \a password using the scrypt parameters
 * stored within \a block.
 *
 * If a valid \a progressDialog pointer is given, the operation will use it
 * to publish its progress. Otherwise, it will be ignored.
 *
 * Upon success, the key is put into \a key.
 *
 * \returns Returns \c true on success, and \c false otherwise.
 */

bool CryptUtil::createKeyFromPassword(QByteArray& key, IdentityBlock& block, QString password, QProgressDialog* progressDialog)
{
    QByteArray scryptSalt = QByteArray::fromHex(block.items[4].value.toLocal8Bit());
    int scryptLogNFactor = block.items[5].value.toInt();
    int scryptIterationCount = block.items[6].value.toInt();

    QByteArray tempKey;
    bool ok = CryptUtil::enScryptIterations(
                tempKey,
                password,
                scryptSalt,
                scryptLogNFactor,
                scryptIterationCount,
                progressDialog);

    if (!ok) return false;

    key = tempKey;
    return true;
}


/*!
 * Derives and returns an identity master key (IMK) from the given
 * decrypted identity unlock key \a decryptedIuk (IUK).
 *
 * This is a purpose-bound alias to the \c enHash function.
 *
 * \sa enHash, createIlkFromIuk
 */

QByteArray CryptUtil::createImkFromIuk(QByteArray decryptedIuk)
{
    return enHash(decryptedIuk);
}

/*!
 * Derives and returns an identity lock key (ILK) from the given
 * identity unlock key \a decryptedIuk (IUK).
 *
 * * \sa createImkFromIuk
 */

QByteArray CryptUtil::createIlkFromIuk(QByteArray decryptedIuk)
{
    QByteArray ilk(32, 0);

    crypto_scalarmult_base(reinterpret_cast<unsigned char*>(ilk.data()),
                           reinterpret_cast<const unsigned char*>(decryptedIuk.constData()));

    return ilk;
}

/*!
 * Performs 16 iterations of the SHA256 hash on \a data, with each successive
 * output XORed to form a 1’s complement sum to produce the final result,
 * which is then returned.
 *
 * \sa createImkFromIuk
 */

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

/*!
 * Creates an \c IdentityBlock of type 1, where the identity keys
 * are being derived from \a iuk and encrypted under \a password.
 *
 * If a valid \a progressDialog pointer is given, the operation will use it
 * to publish its progress. Otherwise, it will be ignored.
 */

IdentityBlock CryptUtil::createBlock1(QByteArray iuk, QString password,
                                      QProgressDialog* progressDialog)
{
    bool ok = false;
    QByteArray initVec(12, 0);
    QByteArray randomSalt(16, 0);
    QByteArray key(32, 0);
    int iterationCount;

    // Generate the random values we'll need
    getRandomBytes(initVec);
    getRandomBytes(randomSalt);

    // Generate IMK, ILK and rescue code
    QByteArray imk = createImkFromIuk(iuk);
    QByteArray ilk = createIlkFromIuk(iuk);

    //TODO: Add error handling

    // Derive key from password
    if (progressDialog != nullptr) progressDialog->setLabelText(
                QObject::tr("Encrypting block 1..."));
    ok = enScryptTime(key, iterationCount, password, randomSalt, 9, 5, progressDialog);

    IdentityBlock block1 = IdentityParser::createEmptyBlock(1);
    block1.items[0].value = "125"; // Length
    block1.items[1].value = "1";   // Type
    block1.items[2].value = "45";  // Plain text length
    block1.items[3].value = initVec.toHex();  // AES GCM initialization vector
    block1.items[4].value = randomSalt.toHex();  // Scrypt random salt
    block1.items[5].value = "9";  // Scrypt log-n-factor
    block1.items[6].value = QString::number(iterationCount);  // Scrypt iterations
    block1.items[7].value = "499";  // Option flags
    block1.items[8].value = "4";  // QuickPass length
    block1.items[9].value = "5";  // Password verify seconds
    block1.items[10].value = "15";  // QuickPass timeout

    // Encrypt identity keys
    QByteArray unencryptedKeys = imk + ilk;
    QByteArray additionalData;
    for (size_t i=0; i<11; i++) additionalData.append(block1.items[i].toByteArray());
    QByteArray encryptedData = aesGcmEncrypt(unencryptedKeys, additionalData, initVec, key);
    QByteArray encryptedImk = encryptedData.left(32);
    QByteArray encryptedIlk = encryptedData.mid(32, 32);
    QByteArray authTag = encryptedData.right(16);

    block1.items[11].value = encryptedImk.toHex();  // IMK
    block1.items[12].value = encryptedIlk.toHex();  // ILK
    block1.items[13].value = authTag.toHex();  // Authentication tag

    return block1;
}

/*!
 * Creates an \c IdentityBlock of type 2 using \a iuk as the identity
 * unlock key and encrypted under \a rescueCode.
 *
 * If a valid \a progressDialog pointer is given, the operation will use it
 * to publish its progress. Otherwise, it will be ignored.
 */

IdentityBlock CryptUtil::createBlock2(QByteArray iuk, QString rescueCode, QProgressDialog *progressDialog)
{
    bool ok = false;
    QByteArray initVec(12, 0);
    QByteArray randomSalt(16, 0);
    QByteArray key(32, 0);
    QByteArray additionalData;
    int iterationCount;

    getRandomBytes(randomSalt);

    IdentityBlock block2 = IdentityParser::createEmptyBlock(2);
    block2.items[0].value = "73"; // Length
    block2.items[1].value = "2";   // Type
    block2.items[2].value = randomSalt.toHex();  // Scrypt random salt
    block2.items[3].value = "9";  // Scrypt log-n-factor

    // Derive key from rescue code
    if (progressDialog != nullptr) progressDialog->setLabelText(
                QObject::tr("Encrypting block 2..."));

    ok = enScryptTime(key, iterationCount, rescueCode, randomSalt, 9, 5, progressDialog);

    block2.items[4].value = QString::number(iterationCount);

    // Encrypt IUK
    for (size_t i=0; i<5; i++) additionalData.append(block2.items[i].toByteArray());
    QByteArray encryptedData = aesGcmEncrypt(iuk, additionalData, initVec, key);
    QByteArray encryptedIuk = encryptedData.left(32);
    QByteArray authTag = encryptedData.right(16);

    block2.items[5].value = encryptedIuk.toHex();  // Encrypted IUK
    block2.items[6].value = authTag.toHex();  // Verification tag

    return block2;
}

/*!
 * \brief Re-encrypts and re-authenticates a type 1 block after a change to
 * the plain-text settings or password.
 *
 * This function will decrypt the IMK and ILK of the (unmodified) \a oldBlock
 * using \a oldPassword and subsequently re-encrypt and re-authenticate them
 * within \a updatedBlock using \a newPassword.
 *
 * If no password change is required, just pass in an identical password to
 * \a oldPassword and \a newPassword.
 *
 * \a updatedBlock must be a copy of \a oldBlock but shall already contain the
 * required plain text changes, which are passed in as "additional data" to the
 * AES GCM authenticated encryption. This function will create a new AES-GCM
 * initialization vector as well as a new scrypt random salt and replace the
 * existing values in \a updatedBlock.
 *
 * If a valid \a progressDialog pointer is given, the operation will use it
 * to publish its progress. Otherwise, it will be ignored.
 *
 * If successful, \a updatedBlock will contain the re-encrypted and
 * re-authenticated data.
 *
 * \return Returns true if the operation succeeds, false otherwise.
 */

bool CryptUtil::updateBlock1(IdentityBlock *oldBlock, IdentityBlock* updatedBlock,
                             QString oldPassword, QString newPassword, QProgressDialog* progressDialog)
{
    QByteArray oldImk(32, 0);
    QByteArray oldIlk(32, 0);
    QByteArray newImk(32, 0);
    QByteArray newIlk(32, 0);
    QByteArray newIv(12, 0);
    QByteArray newPlainText;
    QByteArray newRandomSalt(16, 0);
    QByteArray newKey(32, 0);
    int newIterationCount;

    if (sodium_init() < 0) return false;

    if (progressDialog != nullptr)
        progressDialog->setLabelText(QObject::tr("Decrypting identity keys..."));

    QByteArray key;
    bool ok = CryptUtil::createKeyFromPassword(key, *oldBlock, oldPassword, progressDialog);
    if (!ok) return false;

    ok = decryptBlock1(oldImk, oldIlk, oldBlock, key);
    if (!ok) return false;

    // Re-run the PBKDF with the new parameters
    getRandomBytes(newRandomSalt);
    int scryptLogNFactor = updatedBlock->items[5].value.toInt();
    int passwordVerifySeconds = updatedBlock->items[9].value.toInt();

    if (progressDialog != nullptr)
        progressDialog->setLabelText(QObject::tr("Running PBKDF and re-encrypting identity..."));

    ok = enScryptTime(newKey,
                      newIterationCount,
                      newPassword,
                      newRandomSalt,
                      scryptLogNFactor,
                      passwordVerifySeconds,
                      progressDialog);

    if (!ok) return false;

    getRandomBytes(newIv);

    updatedBlock->items[3].value = newIv.toHex();
    updatedBlock->items[4].value = newRandomSalt.toHex();
    updatedBlock->items[6].value = QString::number(newIterationCount);

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
                reinterpret_cast<const unsigned char*>(newKey.constData()));

    newImk = encryptedKeys.left(32);
    newIlk = encryptedKeys.right(32);

    updatedBlock->items[11].value = newImk.toHex();
    updatedBlock->items[12].value = newIlk.toHex();
    updatedBlock->items[13].value = authTag.toHex();

    return true;
}

/*!
 * Encrypts \a message under the secret key \a key and
 * the initialization vector \a iv, adding in \a additionalData
 * as additional data for the authenticated encryption.
 *
 * The returned data contains the encrypted ciphertext as well
 * as the generated authentication tag, which is being appended
 * right after the ciphertext.
 */

QByteArray CryptUtil::aesGcmEncrypt(QByteArray message, QByteArray additionalData,
                                    QByteArray iv, QByteArray key)
{
     unsigned long long len = message.length() + 16;
    QByteArray cipherText(len, 0);

    crypto_aead_aes256gcm_encrypt(
                reinterpret_cast<unsigned char*>(cipherText.data()),
                reinterpret_cast<unsigned long long*>(&len),
                reinterpret_cast<const unsigned char*>(message.constData()),
                static_cast<unsigned long long>(message.length()),
                reinterpret_cast<const unsigned char*>(additionalData.constData()),
                static_cast<unsigned long long>(additionalData.length()),
                nullptr,
                reinterpret_cast<const unsigned char*>(iv.constData()),
                reinterpret_cast<const unsigned char*>(key.constData()));

    return cipherText;
}

/*!
 * Generates and returns a random 256 bit identity unlock key (IUK).
 * Returns \c nullptr if the operation fails.
 */

QByteArray CryptUtil::createIuk()
{
    QByteArray iuk(32, 0);
    if (!getRandomBytes(iuk)) return nullptr;
    return iuk;
}


/*!
 * Generates and returns a random, 24 digit rescue code string.
 * Returns \c nullptr if the operation fails.
 */

QString CryptUtil::createNewRescueCode()
{
    QByteArray tempBytes(24, 0);
    unsigned char temp;

    if (sodium_init() < 0) return nullptr;

    for (int i=0; i<tempBytes.count(); i+=2)
    {
        temp = 255;
        while (temp > 199)
        {
            if (!getRandomByte(temp)) return nullptr;
        }

        int n = temp % 100;
        tempBytes[i+0] = '0' + static_cast<char>(n/10);
        tempBytes[i+1] = '0' + (n%10);
    }

    return QString(tempBytes);
}


/*!
 * Returns a formatted version of the given \a rescueCode
 * string by inserting a dash after each 4th charcacter.
 * If the given \a rescueCode does not have exactly 24 characters,
 * it is being returned unaltered. Otherwise, the returned
 * string will have the format ("1111-1111-1111-1111-1111-1111").
 */

QString CryptUtil::formatRescueCode(QString rescueCode)
{
    if (rescueCode.length() != 24)
        return rescueCode;

    for (int i=20; i>0; i-=4) rescueCode.insert(i, '-');
    return rescueCode;
}

/*!
 * Creates a new SQRL identity, which is encrypted using \a password.
 *
 * If a valid \a progressDialog pointer is given, the operation will use it
 * to publish its progress. Otherwise, it will be ignored.
 *
 * If successful, a valid \c IdentityModel is placed in \a identity and the
 * rescue code is placed in \a rescueCode.
 *
 * \return Returns \c true on success, \c false otherwise (e.g. if any of
 * the crypotgraphic operations failed or the operation was cancelled by the
 * user using the \c QProgressDialog 's "Cancel" button).
 */

bool CryptUtil::createIdentity(IdentityModel& identity, QString &rescueCode,
                               QString password, QProgressDialog *progressDialog)
{
    rescueCode = createNewRescueCode();
    QByteArray iuk = createIuk();

    // Block 1
    IdentityBlock block1 = createBlock1(iuk, password, progressDialog);
    identity.blocks.push_back(block1);

    // Block 2
    IdentityBlock block2 = createBlock2(iuk, rescueCode, progressDialog);
    identity.blocks.push_back(block2);

    return true;
}

/*!
 * Reversers the given byte array and returns the result.
 * e.g \c "\0x00\0xFF" becomes \c "\0xFF\0x00"
 */

QByteArray CryptUtil::reverseByteArray(QByteArray source)
{
    QByteArray result;
    result.reserve(source.size());

    for (int i=source.size()-1; i>=0; i--)
        result.append(source.at(i));

    return result;
}

/*!
 * Converts a raw byte array into a \c BigUnsigned unsigned integer
 * representation using Matt McCutchen's C++ Big Integer Library at
 * https://mattmccutchen.net/bigint/ .
 */

BigUnsigned CryptUtil::convertByteArrayToBigUnsigned(QByteArray data)
{
    BigUnsigned num(0), exp(1);

    for (int i=0; i<data.count(); i++)
    {
        for (int j=0; j<8; j++)
        {
            if ((data.at(i) >> j) & 1) num += exp;
            exp *= 2;
        }
    }

    return num;
}

/*!
 * Converts a \c BigUnsigned unsigned integer into a raw byte array.
 */

QByteArray CryptUtil::convertBigUnsignedToByteArray(BigUnsigned bigNum)
{
    QByteArray result;
    BigUnsigned quotient(0);
    int bitIndex = 0;
    char currentByte = 0;
    bool isDirty = false;

    while (!bigNum.isZero())
    {
        if (bitIndex == 8)
        {
            result.append(currentByte);
            currentByte = 0;
            bitIndex = 0;
            isDirty = false;
        }

        bigNum.divideWithRemainder(2, quotient); // bigNum now holds remainder

        if (bigNum.toInt() == 1) currentByte |= 1 << bitIndex;

        isDirty = true;
        bigNum = quotient;
        bitIndex++;
    }

    if (isDirty) result.append(currentByte);

    return result;
}

/*!
 * Creates a base-56-encoded textual representation of the provided
 * binary \a identityData.
 *
 * See page 27 of https://www.grc.com/sqrl/SQRL_Cryptography.pdf for
 * more information.
 *
 * \sa base56DecodeIdentity
 */

QString CryptUtil::base56EncodeIdentity(QByteArray identityData)
{
    QByteArray textualId;
    QByteArray checksumBytes;
    int charsOnLine = 0;
    char lineNr = 0;
    BigUnsigned result(0);

    int maxLength = static_cast<int>(
                ceil((identityData.count()*8)/(log(BASE56_BASE_NUM)/log(2)))
                );

    BigUnsigned bigNumber = convertByteArrayToBigUnsigned(identityData);

    for (int i=0; i<maxLength; i++)
    {
        if(charsOnLine == BASE56_LINE_MAX_CHARS) // add check character
        {
            checksumBytes.append(lineNr);
            textualId.append(createBase56CheckSumChar(checksumBytes));
            checksumBytes.clear();
            lineNr++;
            charsOnLine = 0;
        }

        if (bigNumber.isZero()) // zero padding
        {
            textualId.append(BASE56_ALPHABET[0]);
            checksumBytes.append(BASE56_ALPHABET[0]);
        }
        else
        {
            bigNumber.divideWithRemainder(BASE56_BASE_NUM, result); // bigNumber now holds remainder
            textualId.append(BASE56_ALPHABET[bigNumber.toInt()]);
            checksumBytes.append(BASE56_ALPHABET[bigNumber.toInt()]);
            bigNumber = result;
        }
        charsOnLine++;
    }

    checksumBytes.append(lineNr);
    textualId.append(createBase56CheckSumChar(checksumBytes));

    return textualId;
}

/*!
 * Creates a checksum character from the given \a dataBytes
 * according to the SQRL specification for creating/verifying
 * the textual identity version. See "Textual Identity Format"
 * in https://www.grc.com/sqrl/SQRL_Cryptography.pdf for more
 * information.
 */

char CryptUtil::createBase56CheckSumChar(QByteArray dataBytes)
{
    crypto_hash_sha256_state state;
    crypto_hash_sha256_init(&state);
    unsigned char hash[crypto_hash_sha256_BYTES];

    crypto_hash_sha256_update(&state,
                              reinterpret_cast<const unsigned char*>(dataBytes.constData()),
                              static_cast<size_t>(dataBytes.length()));
    crypto_hash_sha256_final(&state, hash);

    QByteArray checksum = QByteArray(reinterpret_cast<const char*>(&hash),
                                     crypto_hash_sha256_BYTES);

    BigUnsigned checksumNum = convertByteArrayToBigUnsigned(checksum);
    BigUnsigned remainder = checksumNum % BASE56_BASE_NUM;

    return BASE56_ALPHABET[remainder.toInt()];
}

/*!
 * Decodes the given base-56-encoded \a textualIdentity and if successful
 * returns the decoded binary identity data. If the \a textualIdentity is
 * invalid, \c nullptr will be returned.
 *
 * See page 27 of https://www.grc.com/sqrl/SQRL_Cryptography.pdf for
 * more information.
 *
 * \sa base56EncodeIdentity
 */

QByteArray CryptUtil::base56DecodeIdentity(QString textualIdentity)
{
    if (!verifyTextualIdentity(textualIdentity))
        return nullptr;

    textualIdentity = stripWhitespace(textualIdentity);

    BigUnsigned bigNum(0), powVal(0);

    for (int i=0; i<textualIdentity.length(); i++)
    {
        if ((i+1) % 20 == 0 || i == textualIdentity.length()-1)
            continue; // Skip check characters

        if (powVal.isZero()) powVal = 1;
        else powVal *= BASE56_BASE_NUM;
        int index = BASE56_ALPHABET.indexOf(textualIdentity.at(i));
        BigUnsigned newVal = powVal * index;
        bigNum += newVal;
    }

    return convertBigUnsignedToByteArray(bigNum);
}

/*!
 * Formats the given \a textualIdentity string to groups of 5x4 characters
 * per line, separated by a space.
 *
 * The output looks something like this:
 *
 * jWJX JmD3 hUKQ qcRQ YRis
 * Gnmx uQHs LaE5 vR2f V7At
 * vW5g UQ5m B5kK gBjH 9uJr
 * JQEF T7sm yYm7 USzH tmfu
 * 6ktj U86K Dqdz p8Gh TfZS
 * UA8a G3F
 */

QString CryptUtil::formatTextualIdentity(QString textualIdentity)
{
    QByteArray result;

    for (int i=0; i<textualIdentity.length(); i++)
    {
        result.append(textualIdentity.at(i));
        if ((i+1) % 20 == 0)
        {
            result.append("\r\n");
            continue;
        }
        if ((i+1) % 4 == 0) result.append(" ");
    }

    return result;
}

/*!
 * Verifies if the given \a textualIdentity is a valid base56-encoded
 * SQRL identity by checking the validity of the check characters.
 *
 * \returns Returns \c true if the identity is valid and \c false
 * otherwise.
 */

bool CryptUtil::verifyTextualIdentity(QString textualIdentity)
{
    textualIdentity = stripWhitespace(textualIdentity);

    char lineNr = 0;

    for (int i=0; i<textualIdentity.length(); i+=20)
    {
        QByteArray checksumBytes = textualIdentity.mid(i, 19).toLocal8Bit();
        if (checksumBytes.count() < 19) // last line - remove check char
            checksumBytes.remove(checksumBytes.length()-1, 1);

        checksumBytes.append(lineNr);

        char computedCheckChar = createBase56CheckSumChar(checksumBytes);

        char checkChar = -1;
        if (checksumBytes.count() < 20)
            checkChar = textualIdentity.toLocal8Bit()[textualIdentity.length()-1];
        else
            checkChar = textualIdentity.toLocal8Bit()[i+19];

        if (checkChar != computedCheckChar) return false;

        lineNr++;
    }

    return true;
}

/*!
 * Removes all occurrances of common "whitespace" characters
 * from \a source and returns the result.
 */

QString CryptUtil::stripWhitespace(QString source)
{
    source.replace("\r", "");
    source.replace("\n", "");
    source.replace("\t", "");
    source.replace(" ", "");

    return source;
}

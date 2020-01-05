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

#include <QtCore>
#include "../testutils.h"
#include "../../src/cryptutil.h"

const int BASE56_VECTORS_COUNT = 128;

void createBase56FullFormatVectors()
{
    // If RECREATE_FROM_FILE is true, the input bytes
    // for the base56 encoding are being read from an
    // existing file and merily recreated. If it is false,
    // "fresh" input bytes are generated randomly.

    const bool RECREATE_FROM_FILE = false;

    const QString FILENAME = "base56-vectors.txt";
    QList<QList<QByteArray>> vectors;
    QList<QByteArray> inputs;


    if (RECREATE_FROM_FILE)
    {
        vectors = TestUtils::parseVectorsCsv(FILENAME);

        for (QList<QByteArray> vector : vectors)
        {
            QByteArray input = QByteArray::fromBase64(vector.at(0), QByteArray::Base64UrlEncoding);
            //QByteArray input = QByteArray::fromHex(vector.at(1));

            // We should have at least one input with all zero bytes
            // to make sure zero-padding works as expected
            if (input.length() == 24) input = QByteArray(24, 0);
            inputs.append(input);
        }
    }
    else
    {
        for (int i=1; i<=BASE56_VECTORS_COUNT; i++)
        {
            QByteArray input(i, 0);
            CryptUtil::getRandomBytes(input);

            // We should have at least one input with all zero bytes
            // to make sure zero-padding works as expected
            if (i==24) input = QByteArray(24, 0);

            inputs.append(input);
        }
    }

    QFile file(FILENAME);
    if( !file.open(QIODevice::WriteOnly) ) return;
    QTextStream outputStream(&file);

    outputStream << "Input(base64_url),Input(hex),Base56EncodedOutput\r\n";

    for (QByteArray input : inputs)
    {
        QString result = CryptUtil::base56EncodeIdentity(input);
        outputStream << '"' << input.toBase64(QByteArray::Base64UrlEncoding |
                                              QByteArray::Base64Option::OmitTrailingEquals)
                     << "\","
                     << '"' << input.toHex() << "\","
                     << "\"" << CryptUtil::formatTextualIdentity(result, true) << "\"\r\n";

    }

    file.close();

    std::cout << "createBase56EncodeVectors() finished...\r\n";
}

int main(int argc, char *argv[])
{
    createBase56FullFormatVectors();
    return 0;
}

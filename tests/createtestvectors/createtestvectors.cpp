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
#include "../../cryptutil.h"

const int BASE56_VECTORS_COUNT = 128;

void createBase56Vectors()
{
    QFile file("base56-vectors.txt");
    if( !file.open(QIODevice::WriteOnly) ) return;
    QTextStream outputStream(&file);

    outputStream << "Input(base64_url),Input(hex),Base56EncodedOutput\r\n";

    for (int i=10; i<10+BASE56_VECTORS_COUNT; i++)
    {
        QByteArray input(i, 0);

        // Avoid '\0' at the end of the byte array,
        // which would make proper base56 decoding impossible
        while(true)
        {
            CryptUtil::getRandomBytes(input);
            if (i==0 || input[i-1] != '\0') break;
        }

        QString result = CryptUtil::base56EncodeIdentity(input);
        outputStream << '"' << input.toBase64(QByteArray::Base64UrlEncoding | QByteArray::Base64Option::OmitTrailingEquals) << "\","
                     << '"' << input.toHex() << "\","
                     << "\"" << result << "\"\r\n";

    }
    file.close();

    std::cout << "createBase56EncodeVectors() finished...\r\n";
}

int main(int argc, char *argv[])
{
    createBase56Vectors();
    return 0;
}

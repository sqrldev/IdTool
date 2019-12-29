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

#include "testutils.h"

QList<QList<QByteArray>> TestUtils::parseVectorsCsv(QString fileName, 
	bool skipFirstLine, bool removeQuotes)
{
    QByteArray data;
	QFile input(fileName);
    bool ok = input.open(QIODevice::ReadOnly);
    if (ok)
    {
        data = input.readAll();
        input.close();
    }

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


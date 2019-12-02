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

#ifndef BLOCKDESIGNERDIALOG_H
#define BLOCKDESIGNERDIALOG_H

#include "common.h"
#include "identityparser.h"
#include "itemeditordialog.h"

namespace Ui {
class BlockDesignerDialog;
}

class BlockDesignerDialog : public QDialog
{
    Q_OBJECT

public:
    enum WorkMode
    {
        ADD,
        EDIT
    };

public:
    explicit BlockDesignerDialog(int blockType, QWidget *parent = nullptr);
    ~BlockDesignerDialog();

private:
    Ui::BlockDesignerDialog *ui;
    WorkMode m_WorkMode;
    int m_BlockType = -1;
    QStandardItemModel* m_pItemModel = nullptr;
    QJsonDocument* m_pBlockDesign = nullptr;

private:
    void createModelStub();
    void createBlockDefinition();
    bool loadBlockDefinition();
    void reload(bool reloadBlockDefinition);

public slots:
    void onAddItemClicked();
    void onDeleteItemClicked();
    void onMoveItemClicked();
    void onEditItemClicked();
    void onSaveButtonClicked();
    void onResetButtonClicked();
};

#endif // BLOCKDESIGNERDIALOG_H

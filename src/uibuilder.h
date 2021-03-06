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

#ifndef UIBUILDER_H
#define UIBUILDER_H

#include "common.h"
#include "identitymodel.h"
#include "identityparser.h"

namespace Ui {
    class MainWindow;
}

/**********************************************
 *    class UiBuilder                         *
 *********************************************/

class UiBuilder : public QObject
{    
    Q_OBJECT

private:
    QScrollArea* m_pContentRoot = nullptr;
    QWidget* m_pLastWidget = nullptr;
    QLayout* m_pLastLayout = nullptr;
    IdentityModel* m_pModel = nullptr;
    bool m_bNeedsRebuild = false;
    bool m_bEnableUnauthenticatedChanges = false;

public:
    UiBuilder(QScrollArea* contentRoot, IdentityModel* identityModel);
    void rebuild();
    void clearLayout();
    IdentityModel* getModel();
    void setEnableUnauthenticatedChanges(bool enable, bool rebuild = true);
    static bool showGetBlockTypeDialog(QString* result, bool allowEdit = false);
    static bool showGetRepeatCountDialog(QString itemName, int* result);

private:
    QWidget* createBlock(IdentityBlock* block);
    QWidget* createBlockHeader(IdentityBlock *block);
    QWidget* createBlockItem(IdentityBlockItem* item, IdentityBlock* block = nullptr);

signals:
    void identityChanged();

public slots:
    void onEditButtonClicked();
    void onCopyButtonClicked();
    void onBlockOptionsButtonClicked();
    void onItemOptionsButtonClicked();
    void onInsertBlock();
    void onDeleteBlock();
    void onMoveBlock();
    void onCopyBlock();
    void onPasteBlock();
    void onInsertItem();
    void onDeleteItem();
    void onMoveItem();
};

/**********************************************
 *    class BlockConnector                    *
 *********************************************/

class BlockConnector
{
public:
    IdentityBlock* block = nullptr;
    bool moveUp = true;

public:
    BlockConnector();
    BlockConnector(const BlockConnector& other);
    BlockConnector(IdentityBlock* block, bool moveUp = true);
    ~BlockConnector();
};

/**********************************************
 *    class ItemConnector                     *
 *********************************************/

class ItemConnector
{
public:
    IdentityBlock* block = nullptr;
    IdentityBlockItem* item = nullptr;
    QLineEdit* valueLabel = nullptr;
    bool moveUp = true;

public:
    ItemConnector();
    ItemConnector(const ItemConnector& other);
    ItemConnector(IdentityBlock* block, IdentityBlockItem* item, QLineEdit* valueLabel, bool moveUp = true);
    ~ItemConnector();
};

Q_DECLARE_METATYPE(BlockConnector);
Q_DECLARE_METATYPE(ItemConnector);

#endif // UIBUILDER_H

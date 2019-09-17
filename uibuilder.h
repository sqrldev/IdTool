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

#include <QMainWindow>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QUuid>
#include <QObjectUserData>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QToolTip>
#include <QPicture>
#include <QMenu>
#include <QMetaType>
#include <QVariant>
#include <QDir>
#include "identitymodel.h"
#include "identityparser.h"

class UiBuilder : public QObject
{    
    Q_OBJECT

public:
    UiBuilder(QScrollArea* scrollArea, IdentityModel* model);
    void rebuild();
    void clear();
    static bool showGetBlockTypeDialog(QString* result);
    static bool showGetRepeatCountDialog(QString itemName, int* result);

private:
    QScrollArea* m_pScrollArea = nullptr;
    QWidget* m_pLastWidget = nullptr;
    QLayout* m_pLastLayout = nullptr;
    IdentityModel* m_pModel = nullptr;

    QWidget* createBlock(IdentityBlock* block);
    QWidget* createBlockHeader(IdentityBlock *block);
    QWidget* createBlockItem(IdentityBlockItem* item);

public slots:
    void editButtonClicked();
    void copyButtonClicked();
    void blockOptionsButtonClicked();
    void itemOptionsButtonClicked();
    void addBlock();
    void deleteBlock();
    void moveBlock();
    void addNewItem();
    void deleteItem();
    void moveItem();
};

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

class ItemConnector
{
public:
    IdentityBlockItem* item = nullptr;
    QLineEdit* valueLabel = nullptr;

public:
    ItemConnector();
    ItemConnector(const ItemConnector& other);
    ItemConnector(IdentityBlockItem* item, QLineEdit* valueLabel);
    ~ItemConnector();
};

Q_DECLARE_METATYPE(BlockConnector);
Q_DECLARE_METATYPE(ItemConnector);

#endif // UIBUILDER_H

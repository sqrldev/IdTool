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

#include "uibuilder.h"
#include "identityclipboard.h"

/*!
 *
 * \class UiBuilder
 * \brief A class for dynamically visualizing a SQRL identity within
 * the application's \c MainWindow GUI.
 *
 * \c UiBuilder, upon being provided with an \c IdentityModel object,
 * will dynamically render the identity's block structure by creating
 * all the necessary UI controls and stylesheets "on the fly" and
 * inserting them into the \c MainWindow GUI.
 *
 * \sa MainWindow, IdentityModel
 *
*/


/*!
 * Creates a new UiBuilder object, storing a pointer to the \a contentRoot
 * \c QScrollArea widget, which acts as the "canvas" onto which the identity
 * representation is being drawn, and the \a identityModel representing
 * the identity structure to be visualized.
 */

UiBuilder::UiBuilder(QScrollArea* contentRoot, IdentityModel* identityModel)
{
    m_pContentRoot = contentRoot;
    m_pContentRoot->setStyleSheet("QScrollArea{border: 0px;}");
    m_pContentRoot->setWidgetResizable(true);
    m_pModel = identityModel;
}

/*!
 * Rebuilds the visual representation of the stored identity model.
 *
 * \note This method is usually called if changes to the underlying
 * identity model should be reflected in the UI representation.
 *
 * \throws A \c std::runtime_error is thrown if either the
 * parent widget onto which the identity should be drawn or the
 * identity model are invalid.
 */

void UiBuilder::rebuild()
{
    if (!m_pContentRoot || !m_pModel)
    {
        throw std::runtime_error(tr("Invalid container or model pointers!")
                                 .toStdString());
    }

    clearLayout();

    QWidget* pWidget = new QWidget();
    QVBoxLayout *pLayout = new QVBoxLayout();
    pLayout->setSpacing(10);

    for (size_t i=0; i<m_pModel->blocks.size(); i++)
    {
        QWidget* pBlock = createBlock(&m_pModel->blocks[i]);
        pLayout->addWidget(pBlock);
    }

    pLayout->addStretch();

    pWidget->setLayout(pLayout);
    m_pContentRoot->setWidget(pWidget);

    m_pLastWidget = pWidget;
    m_pLastLayout = pLayout;

    m_bNeedsRebuild = false;
}

/*!
 * Deletes the layout representing the identity from the UI.
 */

void UiBuilder::clearLayout()
{
    if (m_pLastLayout) m_pLastLayout->deleteLater();
}

/*!
 * Returns a pointer to the \c IdentityModel currently active
 * in this \c UiBuilder instance.
 */

IdentityModel *UiBuilder::getModel()
{
    return m_pModel;
}

/*!
 * \brief Sets an internal flag that signals whether unauthenticated changes
 * to the identity should be enabled or disabled.
 *
 * Any settings and values within a SQRL identitiy are being protected against
 * unauthorized manipulation by applying the AES-GCM authenticated encryption.
 * With this method, the plain text within an identity is included with the
 * encryption of the identity keys as "additional data". Therefore, if any
 * information within an identity changes, the decryption of the keys will fail.
 *
 * "Unauthenticated changes" are changes to the identity which don't get
 * cryptographically "reauthenticated". This means, any well behaving client
 * will reject such "rogue" identities.
 *
 * If \a enable is \c true, unauthenticated changes are being enabled,
 * if it is set to \c false, they are being disabled.
 *
 * If \a rebuild is set to \c true, any change is immediately reflected
 * within the visual representation of the identity. For example, if
 * unauthenticated changes are being disabled and \a rebuild is \c true,
 * buttons for editing and moving identity blocks and items are removed
 * from the GUI.
 */

void UiBuilder::setEnableUnauthenticatedChanges(bool enable, bool rebuild)
{
    m_bEnableUnauthenticatedChanges = enable;
    if (rebuild) this->rebuild();
}

/*!
 * Displays a dialog, asking the user to select or enter a block type.
 *
 * The dialog provides a list of existing block types to choose from
 * (meaning block types that do have a block definition file present).
 * If \a allowEdit is \c true, the user can enter a block type that
 * does not exist, allowing for the creation of new block type definitions.
 * If \a allowEdit is \c false, the user can only choose from the list of
 * existing block types.
 *
 * If the dialog was not cancelled and the user chose or entered a block
 * type, \c true is returned and the block type is put into \a result.
 * Otherwise, \c false is returned.
 */

bool UiBuilder::showGetBlockTypeDialog(QString *result, bool allowEdit)
{
    QDir currentPath = QDir::currentPath();
    QDir fullPath = currentPath.filePath(QString("blockdef/"));
    QStringList fileNames = fullPath.entryList(
                QStringList() << "*.json" << "*.JSON",
                QDir::Files);

    QStringList blockDefs;
    foreach(QString fileName, fileNames) {
        blockDefs.append(
                    fileName.mid(0, fileName.indexOf('.'))
                    );
    }

    bool ok = false;
    QString sType = QInputDialog::getItem(
                nullptr,
                tr("Choose block type"),
                tr("Block type:"),
                blockDefs,
                0,
                allowEdit,
                &ok);

    if (ok) *result = sType;
    return ok;
}

/*!
 * Creates a visual representation of the \c IdentityBlock \a block and returns
 * it as a pointer to a \c QWidget object.
 */

QWidget* UiBuilder::createBlock(IdentityBlock *block)
{
    QString objectName = "obj_" + QUuid::createUuid().toByteArray().toHex();

    QFrame* pFrame = new QFrame();
    pFrame->setObjectName(objectName);
    pFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    QString styleSheet = QString("QFrame#") + objectName +
            " { background: " +
            block->color +
            "; border-radius: 6px; }";
    pFrame->setStyleSheet(styleSheet);
    QGridLayout* pFrameLayout = new QGridLayout();
    pFrameLayout->setSpacing(1);

    QWidget* pBlockHeader = createBlockHeader(block);
    pFrameLayout->addWidget(pBlockHeader);

    // Add items
    for (size_t i=0; i<block->items.size(); i++)
    {
        QWidget* pItemWidget = createBlockItem(&block->items[i], block);
        pFrameLayout->addWidget(pItemWidget);
    }

    pFrame->setLayout(pFrameLayout);

    return pFrame;
}

/*!
 * Creates a visual block header using the information provided
 * within \a block, displaying the block type and description as
 * well as user controls for block manipulation, and returns it
 * as a pointer to a \c QWidget object.
 */

QWidget* UiBuilder::createBlockHeader(IdentityBlock *block)
{
    QWidget* pWidget = new QWidget();
    QHBoxLayout* pLayout = new QHBoxLayout();

    pLayout->setContentsMargins(5,10,5,30);

    QLabel* pBlockDescLabel = new QLabel(block->description);
    QFont font = pBlockDescLabel->font();
    font.setPointSize(14);
    pBlockDescLabel->setFont(font);
    pBlockDescLabel->setWordWrap(true);
    pLayout->addWidget(pBlockDescLabel);

    if (m_bEnableUnauthenticatedChanges)
    {
        QPushButton* pBlockOptionsButton = new QPushButton();
        pBlockOptionsButton->setToolTip(tr("Block options"));
        pBlockOptionsButton->setMaximumWidth(30);
        pBlockOptionsButton->setMinimumWidth(30);
        pBlockOptionsButton->setIcon(QIcon(":/res/img/OptionsDropdown_16x.png"));
        QVariant blockConnectorContainer = QVariant::fromValue(BlockConnector(block));
        pBlockOptionsButton->setProperty("0", blockConnectorContainer );
        connect(pBlockOptionsButton, SIGNAL(clicked()), this, SLOT(onBlockOptionsButtonClicked()));
        pLayout->addWidget(pBlockOptionsButton);
    }

    pWidget->setLayout(pLayout);

    return pWidget;
}

/*!
 * Creates a visual representation of \a item, displaying the item name,
 * description and value as well as user controls for item manipulation,
 * and returns it as a pointer to a \c QWidget object.
 *
 * Within \a block, the item's parent \c IdentityBlock must be provided.
 */

QWidget* UiBuilder::createBlockItem(IdentityBlockItem* item, IdentityBlock* block)
{
    QWidget* pWidget = new QWidget();
    QHBoxLayout* pLayout = new QHBoxLayout();
    QString value = item->value;

    if (value.length() > 50)
    {
        value = value.left(18) + "..." + value.right(18);
    }

    pLayout->setContentsMargins(0,0,0,0);

    QLabel* pDescImageLabel = new QLabel();
    pDescImageLabel->setToolTip(item->description);
    pDescImageLabel->setMaximumWidth(30);
    pDescImageLabel->setMinimumWidth(30);
    QPixmap mypix (":/res/img/InfoRule_16x.png");
    pDescImageLabel->setPixmap(mypix);
    pLayout->addWidget(pDescImageLabel);

    QLabel* pNameLable = new QLabel(item->name);
    pNameLable->setWordWrap(true);
    pNameLable->setMaximumWidth(150);
    pNameLable->setMinimumWidth(150);
    pLayout->addWidget(pNameLable);

    QLineEdit* pValueLineEdit = new QLineEdit(value);
    pValueLineEdit->setToolTip(item->value);
    pValueLineEdit->setToolTipDuration(-1);
    pValueLineEdit->setObjectName("wDataLabel");
    pValueLineEdit->setStyleSheet("QLineEdit#wDataLabel { background: rgb(237, 237, 237); border-radius: 6px; }");
    pValueLineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    pValueLineEdit->setTextMargins(5, 0, 5, 0);
    pValueLineEdit->setReadOnly(true);
    if (item->value.length() > 0) pValueLineEdit->setCursorPosition(0);
    pLayout->addWidget(pValueLineEdit);

    ItemConnector ic(block, item, pValueLineEdit);
    QVariant itemConnectorContainer = QVariant::fromValue(ic);

    QPushButton* pCopyButton = new QPushButton();
    pCopyButton->setToolTip(tr("Copy to clipboard"));
    pCopyButton->setMaximumWidth(30);
    pCopyButton->setMinimumWidth(30);
    pCopyButton->setIcon(QIcon(":/res/img/CopyToClipboard_16x.png"));
    pCopyButton->setProperty("0", itemConnectorContainer);
    connect(pCopyButton, SIGNAL(clicked()), this, SLOT(onCopyButtonClicked()));
    pLayout->addWidget(pCopyButton);

    if (m_bEnableUnauthenticatedChanges)
    {
        QPushButton* pEditButton = new QPushButton();
        pEditButton->setToolTip(tr("Edit value"));
        pEditButton->setMaximumWidth(30);
        pNameLable->setMinimumWidth(30);
        pEditButton->setIcon(QIcon(":/res/img/Edit_16x.png"));
        pEditButton->setProperty("0", itemConnectorContainer);
        connect(pEditButton, SIGNAL(clicked()), this, SLOT(onEditButtonClicked()));
        pLayout->addWidget(pEditButton);

        QPushButton* pOptionsButton = new QPushButton();
        pOptionsButton->setToolTip(tr("Item options"));
        pOptionsButton->setMaximumWidth(30);
        pOptionsButton->setMinimumWidth(30);
        pOptionsButton->setIcon(QIcon(":/res/img/OptionsDropdown_16x.png"));
        pOptionsButton->setProperty("0", itemConnectorContainer);
        connect(pOptionsButton, SIGNAL(clicked()), this, SLOT(onItemOptionsButtonClicked()));
        pLayout->addWidget(pOptionsButton);
    }

    pWidget->setLayout(pLayout);

    return pWidget;
}

/*!
 * Displays a dialog, asking the user to enter the number of repititions
 * for the item named \a itemName.
 *
 * If the dialog was not cancelled and the user entered a valid repitition
 * count, \c true is returned and the repitition count is put into \a result.
 * Otherwise, \c false is returned.
 */

bool UiBuilder::showGetRepeatCountDialog(QString itemName, int* result)
{
    bool ok;
    int repititons = QInputDialog::getInt(
                nullptr,
                tr("Item repitition"),
                tr("How many \"%1\" fields should be created?").arg(itemName),
                1,
                1,
                1024,
                1,
                &ok);

    if (ok) *result = repititons;

    return ok;
}


/***************************************************
 *                S L O T S                        *
 * ************************************************/


void UiBuilder::onEditButtonClicked()
{
    ItemConnector connector =
            sender()->property("0").value<ItemConnector>();

    bool ok = false;    
    QString result = QInputDialog::getMultiLineText(
                nullptr,
                tr("Edit value"),
                tr("New value for \"%1\":").arg(connector.item->name),
                connector.item->value, &ok);

    if (ok)
    {
        connector.item->value = result;
        connector.valueLabel->setText(result);

        identityChanged();
    }
}

void UiBuilder::onCopyButtonClicked()
{
    ItemConnector connector =
            sender()->property("0").value<ItemConnector>();

        QClipboard* pClipboard = QApplication::clipboard();
        pClipboard->setText(connector.item->value);

        QToolTip::showText(
                    static_cast<QWidget*>(sender())->mapToGlobal(QPoint(0,0)),
                    tr("Value copied to clipboard!"),
                    static_cast<QWidget*>(sender()));
}

void UiBuilder::onBlockOptionsButtonClicked()
{
    BlockConnector connector =
            sender()->property("0").value<BlockConnector>();

    QAction* pActionMoveBlockUp = new QAction(QIcon(":/res/img/DoubleUp_24x.png"), tr("Move up"));
    QVariant upBlockConnectorContainer = QVariant::fromValue(BlockConnector(connector.block, true));
    pActionMoveBlockUp->setProperty("0", upBlockConnectorContainer);

    QAction* pActionMoveBlockDown = new QAction(QIcon(":/res/img/DoubleDown_24x.png"), tr("Move down"));
    QVariant downBlockConnectorContainer = QVariant::fromValue(BlockConnector(connector.block, false));
    pActionMoveBlockDown->setProperty("0", downBlockConnectorContainer);

    QAction* pActionSeparator = new QAction();
    pActionSeparator->setSeparator(true);

    QAction* pActionCopyBlock = new QAction(QIcon(":/res/img/CopyItem_16x.png"), tr("Copy block"));
    QVariant copyBlockConnectorContainer = QVariant::fromValue(BlockConnector(connector.block));
    pActionCopyBlock->setProperty("0", copyBlockConnectorContainer);

    QAction* pActionPasteBlock = new QAction(QIcon(":/res/img/Paste_16x.png"), tr("Paste block"));
    QVariant pasteBlockConnectorContainer = QVariant::fromValue(BlockConnector(connector.block));
    pActionPasteBlock->setProperty("0", pasteBlockConnectorContainer);
    pActionPasteBlock->setEnabled(IdentityClipboard::getInstance()->hasBlock());

    QAction* pActionSeparator2 = new QAction();
    pActionSeparator2->setSeparator(true);

    QAction* pActionAddBlock = new QAction(QIcon(":/res/img/Add_16x.png"), tr("Add block"));
    QVariant addBlockConnectorContainer = QVariant::fromValue(BlockConnector(connector.block));
    pActionAddBlock->setProperty("0", addBlockConnectorContainer);

    QAction* pActionDeleteBlock = new QAction(QIcon(":/res/img/DeleteBlock_16x.png"), tr("Delete block"));
    QVariant deleteBlockConnectorContainer = QVariant::fromValue(BlockConnector(connector.block));
    pActionDeleteBlock->setProperty("0", deleteBlockConnectorContainer);

    QMenu* menu = new QMenu(static_cast<QWidget*>(sender()));
    menu->addAction(pActionMoveBlockUp);
    menu->addAction(pActionMoveBlockDown);
    menu->addAction(pActionSeparator);
    menu->addAction(pActionCopyBlock);
    menu->addAction(pActionPasteBlock);
    menu->addAction(pActionSeparator2);
    menu->addAction(pActionAddBlock);
    menu->addAction(pActionDeleteBlock);

    connect(pActionMoveBlockUp, &QAction::triggered, this, &UiBuilder::onMoveBlock);
    connect(pActionMoveBlockDown, &QAction::triggered, this, &UiBuilder::onMoveBlock);
    connect(pActionAddBlock, &QAction::triggered, this, &UiBuilder::onInsertBlock);
    connect(pActionDeleteBlock, &QAction::triggered, this, &UiBuilder::onDeleteBlock);
    connect(pActionCopyBlock, &QAction::triggered, this, &UiBuilder::onCopyBlock);
    connect(pActionPasteBlock, &QAction::triggered, this, &UiBuilder::onPasteBlock);

    menu->exec(static_cast<QWidget*>(sender())->mapToGlobal(
                    QPoint(0, 0)));

    if (m_bNeedsRebuild) rebuild();
}

void UiBuilder::onDeleteBlock()
{
    BlockConnector connector =
            sender()->property("0").value<BlockConnector>();

    if (m_pModel->deleteBlock(connector.block))
    {
        m_bNeedsRebuild = true;
        identityChanged();
    }
}

void UiBuilder::onMoveBlock()
{
    BlockConnector connector =
            sender()->property("0").value<BlockConnector>();

    if (m_pModel->moveBlock(connector.block, connector.moveUp))
    {
        m_bNeedsRebuild = true;
        identityChanged();
    }
}

void UiBuilder::onCopyBlock()
{
    BlockConnector connector =
            sender()->property("0").value<BlockConnector>();

    IdentityClipboard* pClipboard = IdentityClipboard::getInstance();
    pClipboard->setBlock(*(connector.block));
}

void UiBuilder::onPasteBlock()
{
    BlockConnector connector =
            sender()->property("0").value<BlockConnector>();

    IdentityClipboard* pClipboard = IdentityClipboard::getInstance();
    if (!pClipboard->hasBlock()) return;
    IdentityBlock block = pClipboard->getBlock();

    if (m_pModel->insertBlock(block, connector.block))
    {
        m_bNeedsRebuild = true;
        identityChanged();
    }
}

void UiBuilder::onInsertBlock()
{
    BlockConnector connector =
            sender()->property("0").value<BlockConnector>();

    bool ok = false;
    QString sBlockType;
    ok = showGetBlockTypeDialog(&sBlockType);
    if (!ok) return;

    ushort blockType = sBlockType.toUShort(&ok);
    if (!ok) return;

    IdentityBlock block = IdentityParser::createEmptyBlock(blockType);

    if (m_pModel->insertBlock(block, connector.block))
    {
        m_bNeedsRebuild = true;
        identityChanged();
    }
}

void UiBuilder::onItemOptionsButtonClicked()
{
    ItemConnector connector =
            sender()->property("0").value<ItemConnector>();

    QAction* pActionMoveItemUp = new QAction(QIcon(":/res/img/DoubleUp_24x.png"), tr("Move up"));
    QVariant upItemConnectorContainer = QVariant::fromValue(
                ItemConnector(connector.block, connector.item, nullptr, true));
    pActionMoveItemUp->setProperty("0", upItemConnectorContainer);

    QAction* pActionMoveItemDown = new QAction(QIcon(":/res/img/DoubleDown_24x.png"), tr("Move down"));
    QVariant downItemConnectorContainer = QVariant::fromValue(
               ItemConnector(connector.block, connector.item, nullptr, false));
    pActionMoveItemDown->setProperty("0", downItemConnectorContainer);

    QAction* pActionSeparator = new QAction();
    pActionSeparator->setSeparator(true);

    QAction* pActionAddItem = new QAction(QIcon(":/res/img/Add_16x.png"), tr("Add item"));
    QVariant addItemConnectorContainer = QVariant::fromValue(
                ItemConnector(connector.block, connector.item, nullptr));
    pActionAddItem->setProperty("0", addItemConnectorContainer);

    QAction* pActionDeleteItem = new QAction(QIcon(":/res/img/DeleteBlock_16x.png"), tr("Delete item"));
    QVariant deleteItemConnectorContainer = QVariant::fromValue(
                ItemConnector(connector.block, connector.item, nullptr));
    pActionDeleteItem->setProperty("0", deleteItemConnectorContainer);

    QMenu* menu = new QMenu(static_cast<QWidget*>(sender()));
    menu->addAction(pActionMoveItemUp);
    menu->addAction(pActionMoveItemDown);
    menu->addAction(pActionSeparator);
    menu->addAction(pActionAddItem);
    menu->addAction(pActionDeleteItem);

    connect(pActionMoveItemUp, &QAction::triggered, this, &UiBuilder::onMoveItem);
    connect(pActionMoveItemDown, &QAction::triggered, this, &UiBuilder::onMoveItem);
    connect(pActionAddItem, &QAction::triggered, this, &UiBuilder::onInsertItem);
    connect(pActionDeleteItem, &QAction::triggered, this, &UiBuilder::onDeleteItem);

    menu->exec(static_cast<QWidget*>(sender())->mapToGlobal(
                    QPoint(0, 0)));

    if (m_bNeedsRebuild) rebuild();
}

void UiBuilder::onDeleteItem()
{
    ItemConnector connector =
            sender()->property("0").value<ItemConnector>();

    bool ok = connector.block->deleteItem(connector.item);

    if (ok)
    {
        m_bNeedsRebuild = true;
        identityChanged();
    }
}

void UiBuilder::onMoveItem()
{
    ItemConnector connector =
            sender()->property("0").value<ItemConnector>();

    if (connector.block->moveItem(connector.item, connector.moveUp))
    {
        m_bNeedsRebuild = true;
        identityChanged();
    }
}

void UiBuilder::onInsertItem()
{
    ItemConnector connector =
            sender()->property("0").value<ItemConnector>();

    bool ok = false;
    QStringList dataTypes = IdentityBlockItem::getDataTypeList();

    QString sDataType = QInputDialog::getItem(
                nullptr,
                tr("Choose data type"),
                tr("Data type for item:"),
                dataTypes,
                0,
                false,
                &ok);

    if (!ok) return;

    ItemDataType dataType = IdentityBlockItem::findDataType(sDataType);

    int nrOfBytes = IdentityBlockItem::DataTypeMap[dataType].nrOfBytes;

    if (nrOfBytes == 0)
    {
        nrOfBytes = QInputDialog::getInt(
                    nullptr,
                    tr("Choose number of bytes"),
                    tr("Number of bytes:"),
                    1,
                    1,
                    2147483647,
                    1,
                    &ok);

        if (!ok) return;
    }

    QString sName = QInputDialog::getText(
                nullptr,
                tr("Choose name"),
                tr("Item name:"),
                QLineEdit::Normal,
                "",
                &ok);

    if (!ok) return;

    QString sDescription = QInputDialog::getText(
                nullptr,
                tr("Choose description"),
                tr("Item description:"),
                QLineEdit::Normal,
                "",
                &ok);

    if (!ok) return;

    IdentityBlockItem item = IdentityParser::createEmptyItem(
                sName, sDescription, dataType, nrOfBytes);

    if (connector.block->insertItem(item, connector.item))
    {
        m_bNeedsRebuild = true;
        identityChanged();
    }
}





/*!
 *
 * \class BlockConnector
 * \brief A helper class for connecting GUI representations of \c IdentityBlock
 * instances to their underlying data source.
 *
 * This helper class is instantiated and populated by the \c UiBuilder class to
 * establish a connection between the individual GUI representation of block objects
 * and their underlying data sources within the \c IdentityModel. To do so,
 * \c UiBuilder will turn the \c BlockConnector instance into a \c QVariant and
 * attach it to the GUI control using \c setProperty(), where it can be retrieved
 * within the slot handling the specific user action which needs access to the
 * data within the \c IdentityModel.
 *
 * \sa UiBuilder, IdentityModel, ItemConnector
 *
*/

BlockConnector::BlockConnector() :
block(nullptr), moveUp(true)
{
}

BlockConnector::BlockConnector(IdentityBlock* block, bool moveUp)
{
    this->block = block;
    this->moveUp = moveUp;
}

BlockConnector::BlockConnector(const BlockConnector& other)
{
    this->block = other.block;
    this->moveUp = other.moveUp;
}

BlockConnector::~BlockConnector()
{
}





/*!
 *
 * \class ItemConnector
 * \brief A helper class for connecting GUI representations of \c IdentityBlockItem
 * instances to their underlying data source.
 *
 * This helper class is instantiated and populated by the \c UiBuilder class to
 * establish a connection between the individual GUI representation of block items
 * and their underlying data sources within the \c IdentityModel. To do so,
 * \c UiBuilder will turn the \c ItemConnector instance into a \c QVariant and
 * attach it to the GUI control using \c setProperty(), where it can be retrieved
 * within the slot handling the specific user action which needs access to the
 * data within the \c IdentityModel.
 *
 * \sa UiBuilder, IdentityModel, BlockConnector
 *
*/

ItemConnector::ItemConnector() :
    block(nullptr), item(nullptr), valueLabel(nullptr), moveUp(true)
{
}

ItemConnector::ItemConnector(IdentityBlock* block, IdentityBlockItem* item, QLineEdit* valueLabel, bool moveUp)
{
    this->block = block;
    this->item = item;
    this->valueLabel = valueLabel;
    this->moveUp = moveUp;
}

ItemConnector::ItemConnector(const ItemConnector& other)
{
    this->block = other.block;
    this->item = other.item;
    this->valueLabel = other.valueLabel;
    this->moveUp = other.moveUp;
}

ItemConnector::~ItemConnector()
{
}

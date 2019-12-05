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

#include "blockdesignerdialog.h"
#include "ui_blockdesignerdialog.h"

/*!
 *
 * \class BlockDesignerDialog
 * \brief A UI dialog window for designing S4 identity storage blocks.
 *
 * \c BlockDesignerDialog offers functionality for visually creating
 * and editing json-templates representing \c IdentityBlock objects
 * using the GUI.
 *
 * \c ItemEditorDialog will be used to edit individual items/settings within
 * the current block.
 *
 * \sa ItemEditorDialog, IdentityModel, IdentitiyBlock, IdentityBlockItem,
 * IdentityParser
 *
*/

/*!
 * \fn BlockDesignerDialog::BlockDesignerDialog(int blockType, QWidget *parent)
 *
 * Creates a new \c BlockDesignerDialog form, using \a parent as the parent
 * form, and setting the type of the block to be edited/created to \a blockType.
 *
 * If a block definition for \a blockType already exists, \c m_WorkMode is set to
 * \c EDIT, enabling editing mode. Otherwise, \c m_WorkMode is set to \c ADD,
 * indicating that a new block definition should be created.
 */

BlockDesignerDialog::BlockDesignerDialog(int blockType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockDesignerDialog)
{
    ui->setupUi(this);
    m_BlockType = blockType;

    if (IdentityParser::hasBlockDefinition(static_cast<uint16_t>(blockType)))
        m_WorkMode = EDIT;
    else m_WorkMode = ADD;

    connect(ui->btnAddItem, &QPushButton::clicked, this, &BlockDesignerDialog::onAddItemClicked);
    connect(ui->btnDeleteItem, &QPushButton::clicked, this, &BlockDesignerDialog::onDeleteItemClicked);
    connect(ui->btnEditItem, &QPushButton::clicked, this, &BlockDesignerDialog::onEditItemClicked);
    connect(ui->btnMoveItemUp, &QPushButton::clicked, this, &BlockDesignerDialog::onMoveItemClicked);
    connect(ui->btnMoveItemDown, &QPushButton::clicked, this, &BlockDesignerDialog::onMoveItemClicked);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onSaveButtonClicked()));
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(onResetButtonClicked()));

    ui->labelHeadline->setText(
                ui->labelHeadline->text().arg(blockType));

    createModelStub();

    if (m_WorkMode == ADD)
    {
        createBlockDefinition();
        QJsonObject test = m_pBlockDesign->object();
        reload(false);
    }
    else if (m_WorkMode == EDIT)
    {
        reload(true);
    }
}

/*!
 * Destructor, frees allocated resources.
 */

BlockDesignerDialog::~BlockDesignerDialog()
{
    delete ui;
    if (m_pItemModel != nullptr) delete m_pItemModel;
    if (m_pBlockDesign != nullptr) delete m_pBlockDesign;
}

/*!
 * Creates a new \c QStandardItemModel, sets the correct header
 * labels and puts a pointer to the model into \c m_pItemModel.
 */

void BlockDesignerDialog::createModelStub()
{
    m_pItemModel = new QStandardItemModel(0, 6);
    m_pItemModel->setHorizontalHeaderLabels(
                QStringList({
                                tr("Name"),
                                tr("Description"),
                                tr("DataType"),
                                tr("NrOfBytes"),
                                tr("RepeatIndex"),
                                tr("RepeatCount")
                            }));
}

/*!
 * Creates a new \c QJsonDocument representing the block definition,
 * asking the user to provide a block description and -color. Finally,
 * a pointer to the block definition is stored within \c m_pBlockDesign.
 *
 * The block's type is set in the constructor of \c BlockDesignerDialog.
 *
 * \sa loadBlockDefinition
 */

void BlockDesignerDialog::createBlockDefinition()
{
    if (m_pBlockDesign != nullptr)
    {
        delete m_pBlockDesign;
    }
    m_pBlockDesign = new QJsonDocument();

    QJsonObject o;
    o["block_type"] = m_BlockType;

    bool ok = false;
    QString sDescription;
    QString sColor;

    while (!ok)
    {
        sDescription = QInputDialog::getText(
                    this,
                    tr("Block type description"),
                    tr("Block type description:"),
                    QLineEdit::Normal,
                    "",
                    &ok);
    }

    o["description"] = sDescription;
    ok = false;

    while (!ok)
    {
        sColor = QInputDialog::getText(
                    this,
                    tr("Block color"),
                    tr("Block color (RGB): example: \"150, 150, 150\""),
                    QLineEdit::Normal,
                    "",
                    &ok);
    }

    o["color"] = "rgb(" + sColor  + ")";
    o["items"] = QJsonArray();

    m_pBlockDesign->setObject(o);
}

/*!
 * Loads an existing json block definition from disk, and places
 * a pointer to the definition into \c m_pBlockDesign.
 *
 * The block type to be loaded is set in the \c BlockDesignerDialog
 * constructor.
 *
 * \returns Returns \c true if a valid block definition could be loaded,
 * and \c false otherwise.
 *
 * \sa createBlockDefinition
 */

bool BlockDesignerDialog::loadBlockDefinition()
{
    if (m_pBlockDesign == nullptr) m_pBlockDesign = new QJsonDocument();

    bool ok = IdentityParser::parseBlockDefinition(
                IdentityParser::getBlockDefinitionBytes(
                    static_cast<uint16_t>(m_BlockType)), m_pBlockDesign);

    if (!ok)
    {
        if (m_pBlockDesign != nullptr)
        {
            delete m_pBlockDesign;
            m_pBlockDesign = nullptr;
        }
        return false;
    }

    return true;
}

/*!
 * Reloads block items from the block definition into the item model,
 * discarding any changes which might have occured through the table view.
 *
 * If \a reloadBlockDefinition is \c true, the block definition gets
 * reloaded from disk prior to reloading the items.
 *
 * \sa loadBlockDefinition
*/

void BlockDesignerDialog::reload(bool reloadBlockDefinition)
{
    if (reloadBlockDefinition)
    {
        if (!loadBlockDefinition()) return;
    }

    m_pItemModel->clear();
    createModelStub();

    QJsonObject rootObj = (*m_pBlockDesign).object();
    QJsonArray items = rootObj["items"].toArray();
    for (QJsonValue item: items)
    {
        QJsonObject itemObj = item.toObject();
        QList<QStandardItem*> stdItemList = IdentityParser::toStandardItems(
                    &itemObj);

        m_pItemModel->appendRow(stdItemList);

    }

    ui->tableView->setModel(m_pItemModel);
    ui->tableView->resizeColumnToContents(0);
}

/***************************************************
 *                S L O T S                        *
 * ************************************************/


void BlockDesignerDialog::onAddItemClicked()
{
    ItemEditorDialog itemEditor(this);
    if (itemEditor.exec() != Accepted) return;

    QJsonObject* item = itemEditor.getItem();
    QJsonObject rootObj = (*m_pBlockDesign).object();

    QJsonArray items = rootObj["items"].toArray();
    items.append(*item);
    QJsonObject temp = (*m_pBlockDesign).object();
    temp.remove("items");
    temp.insert("items", items);
    m_pBlockDesign->setObject(temp);

    reload(false);

    delete item;
}

void BlockDesignerDialog::onDeleteItemClicked()
{
    if (!ui->tableView->model()) return;

    int rowCount = ui->tableView->model()->rowCount();
    if (rowCount == 0) return;

    QItemSelectionModel *selection = ui->tableView->selectionModel();
    if (!selection->hasSelection()) return;

    QModelIndexList selectedRows = selection->selectedRows();
    int rowIndex = selectedRows[0].row();

    QJsonObject rootObj = (*m_pBlockDesign).object();
    QJsonArray items = rootObj["items"].toArray();
    items.removeAt(rowIndex);

    QJsonObject temp = (*m_pBlockDesign).object();
    temp.remove("items");
    temp.insert("items", items);
    m_pBlockDesign->setObject(temp);

    reload(false);

    rowCount = ui->tableView->model()->rowCount();
    if (rowCount > 0)
    {
        if (rowIndex >= rowCount) ui->tableView->selectRow(rowCount-1);
        else ui->tableView->selectRow(rowIndex);
    }
}

void BlockDesignerDialog::onMoveItemClicked()
{
    if (!ui->tableView->model()) return;

    int rowCount = ui->tableView->model()->rowCount();
    if (rowCount == 0) return;

    QItemSelectionModel *selection = ui->tableView->selectionModel();
    if (!selection->hasSelection()) return;

    QModelIndexList selectedRows = selection->selectedRows();
    int rowIndex = selectedRows[0].row();

    QJsonObject rootObj = (*m_pBlockDesign).object();
    QJsonArray items = rootObj["items"].toArray();
    QJsonObject obj = items[rowIndex].toObject();

    if (sender() == ui->btnMoveItemUp)
    {
        if (rowIndex == 0) return;
        items.removeAt(rowIndex);
        items.insert(rowIndex-1, obj);
        rowIndex--;
    }
    else if (sender() == ui->btnMoveItemDown)
    {
        if (rowIndex == items.size()-1) return;
        items.removeAt(rowIndex);
        items.insert(rowIndex+1, obj);
        rowIndex++;
    }

    QJsonObject temp = (*m_pBlockDesign).object();
    temp.remove("items");
    temp.insert("items", items);
    m_pBlockDesign->setObject(temp);

    reload(false);
    ui->tableView->selectRow(rowIndex);
}

void BlockDesignerDialog::onEditItemClicked()
{
    if (!ui->tableView->model()) return;

    int rowCount = ui->tableView->model()->rowCount();
    if (rowCount == 0) return;


    QItemSelectionModel *selection = ui->tableView->selectionModel();
    if (!selection->hasSelection()) return;

    QModelIndexList selectedRows = selection->selectedRows();
    int rowIndex = selectedRows[0].row();

    QJsonObject rootObj = (*m_pBlockDesign).object();
    QJsonArray items = rootObj["items"].toArray();
    QJsonObject item = items.at(rowIndex).toObject();

    ItemEditorDialog itemEditor(this, &item);
    if (itemEditor.exec() != Accepted) return;

    QJsonObject* updatedItem = itemEditor.getItem();
    items.replace(rowIndex, *updatedItem);
    QJsonObject temp = (*m_pBlockDesign).object();
    temp.remove("items");
    temp.insert("items", items);
    m_pBlockDesign->setObject(temp);

    reload(false);
}

void BlockDesignerDialog::onSaveButtonClicked()
{
    QByteArray data = m_pBlockDesign->toJson();

    QDir path = QDir::currentPath();
    QDir fullPath = path.filePath(QString("blockdef/") + QString::number(m_BlockType) + ".json");
    QString sFullPath = fullPath.absolutePath();

    if (QFile::exists(sFullPath))
    {
        QMessageBox::StandardButton result;
          result = QMessageBox::question(
                      this,
                      "File exists",
                      "This file already exists. Are you sure you want to overwrite it?",
                      QMessageBox::Yes|QMessageBox::No);

          if (result != QMessageBox::Yes) return;
    }

    QSaveFile file(sFullPath);

    if (file.open(QIODevice::WriteOnly))
    {
        file.write(data);
        file.commit();
    }
    else
    {
        QMessageBox msgBox(this);
        msgBox.setText(tr("Error writing block definition file!"));
        msgBox.exec();
    }

}

void BlockDesignerDialog::onResetButtonClicked()
{
    if (IdentityParser::hasBlockDefinition(static_cast<uint16_t>(m_BlockType)))
    {
        reload(true);
    }
    else
    {
        createBlockDefinition();
        reload(false);
    }
}

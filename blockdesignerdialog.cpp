#include "blockdesignerdialog.h"
#include "ui_blockdesignerdialog.h"

BlockDesignerDialog::BlockDesignerDialog(int blockType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockDesignerDialog)
{
    ui->setupUi(this);
    m_BlockType = blockType;

    connect(ui->btnAddItem, &QPushButton::clicked, this, &BlockDesignerDialog::addItem);
    connect(ui->btnDeleteItem, &QPushButton::clicked, this, &BlockDesignerDialog::deleteItem);
    connect(ui->btnEditItem, &QPushButton::clicked, this, &BlockDesignerDialog::editItem);
    connect(ui->btnMoveItemUp, &QPushButton::clicked, this, &BlockDesignerDialog::moveItem);
    connect(ui->btnMoveItemDown, &QPushButton::clicked, this, &BlockDesignerDialog::moveItem);

    ui->labelHeadline->setText(
                ui->labelHeadline->text().arg(blockType));

    createModel();
    reload();
}

BlockDesignerDialog::~BlockDesignerDialog()
{
    delete ui;
    if (m_pItemModel != nullptr) delete m_pItemModel;
    if (m_pBlockDesign != nullptr) delete m_pBlockDesign;
}

void BlockDesignerDialog::createModel()
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

bool BlockDesignerDialog::loadBlockDefinition()
{
    if (m_pBlockDesign == nullptr) m_pBlockDesign = new QJsonDocument();

    bool ok = IdentityParser::getBlockDefinition(
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

void BlockDesignerDialog::reload(bool reloadBlockDefinition)
{
    if (reloadBlockDefinition)
    {
        if (!loadBlockDefinition()) return;
    }

    m_pItemModel->clear();
    createModel();

    QJsonArray items = (*m_pBlockDesign)["items"].toArray();
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

void BlockDesignerDialog::addItem()
{

}

void BlockDesignerDialog::deleteItem()
{
    if (!ui->tableView->model()) return;

    int rowCount = ui->tableView->model()->rowCount();
    if (rowCount == 0) return;

    QItemSelectionModel *selection = ui->tableView->selectionModel();
    if (!selection->hasSelection()) return;

    QModelIndexList selectedRows = selection->selectedRows();
    int rowIndex = selectedRows[0].row();

    QJsonArray items = (*m_pBlockDesign)["items"].toArray();
    items.removeAt(rowIndex);

    QJsonObject temp = (*m_pBlockDesign)[0].toObject();
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

void BlockDesignerDialog::moveItem()
{
    if (!ui->tableView->model()) return;

    int rowCount = ui->tableView->model()->rowCount();
    if (rowCount == 0) return;

    QItemSelectionModel *selection = ui->tableView->selectionModel();
    if (!selection->hasSelection()) return;

    QModelIndexList selectedRows = selection->selectedRows();
    int rowIndex = selectedRows[0].row();

    QJsonArray items = (*m_pBlockDesign)["items"].toArray();
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

    QJsonObject temp = (*m_pBlockDesign)[0].toObject();
    temp.remove("items");
    temp.insert("items", items);
    m_pBlockDesign->setObject(temp);

    reload(false);
    ui->tableView->selectRow(rowIndex);
}

void BlockDesignerDialog::editItem()
{
    if (!ui->tableView->model()) return;

    int rowCount = ui->tableView->model()->rowCount();
    if (rowCount == 0) return;


    QItemSelectionModel *selection = ui->tableView->selectionModel();
    if (!selection->hasSelection()) return;

    QModelIndexList selectedRows = selection->selectedRows();
    int rowIndex = selectedRows[0].row();
}

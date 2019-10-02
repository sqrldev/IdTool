#include "blockdesignerdialog.h"
#include "ui_blockdesignerdialog.h"

BlockDesignerDialog::BlockDesignerDialog(int blockType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockDesignerDialog)
{
    ui->setupUi(this);
    m_BlockType = blockType;

    connect(ui->btnDeleteItem, &QPushButton::clicked, this, &BlockDesignerDialog::deleteItem);

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
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    if (!selection->hasSelection()) return;

    QModelIndexList selectedRows = selection->selectedRows();

    QJsonArray items = (*m_pBlockDesign)["items"].toArray();
    items.removeAt(selectedRows[0].row());

    QJsonObject temp = (*m_pBlockDesign)[0].toObject();
    temp.remove("items");
    temp.insert("items", items);

    m_pBlockDesign->setObject(temp);
    reload(false);
}

void BlockDesignerDialog::moveItem()
{


    if (sender() == ui->btnMoveItemUp)
    {

    }
    else if (sender() == ui->btnMoveItemDown)
    {

    }
}

void BlockDesignerDialog::editItem()
{

}

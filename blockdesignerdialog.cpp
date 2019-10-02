#include "blockdesignerdialog.h"
#include "ui_blockdesignerdialog.h"

BlockDesignerDialog::BlockDesignerDialog(int blockType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockDesignerDialog)
{
    ui->setupUi(this);
    m_BlockType = blockType;

    ui->labelHeadline->setText(
                ui->labelHeadline->text().arg(blockType));

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
    loadData();

}

BlockDesignerDialog::~BlockDesignerDialog()
{
    delete ui;
    delete m_pItemModel;
}

void BlockDesignerDialog::loadData()
{
    QJsonDocument jsonDoc;

    bool ok = IdentityParser::getBlockDefinition(
                IdentityParser::getBlockDefinitionBytes(
                    static_cast<uint16_t>(m_BlockType)), &jsonDoc);

    if (!ok) return;

    for (QJsonValue item: jsonDoc["items"].toArray())
    {
        QJsonObject itemObj = item.toObject();
        QList<QStandardItem*> stdItemList = IdentityParser::toStandardItems(
                    &itemObj);

        m_pItemModel->appendRow(stdItemList);

    }

    ui->tableView->setModel(m_pItemModel);
    ui->tableView->resizeColumnToContents(0);
}

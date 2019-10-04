#include "itemeditordialog.h"
#include "ui_itemeditordialog.h"

ItemEditorDialog::ItemEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ItemEditorDialog)
{
    ui->setupUi(this);
}

ItemEditorDialog::ItemEditorDialog(QWidget *parent, QJsonObject *item) :
    ItemEditorDialog(parent)
{
    m_pItem = item;

    loadDefaults();
    if (m_pItem != nullptr) loadItemData();
}

ItemEditorDialog::~ItemEditorDialog()
{
    delete ui;
}

QJsonObject *ItemEditorDialog::getItem()
{
    return m_pItem;
}

void ItemEditorDialog::loadDefaults()
{
    for (auto const& dataTypeItem : IdentityBlockItem::DataTypeMap)
    {
        ui->cmbDataType->addItem(dataTypeItem.second.name);
    }
}

void ItemEditorDialog::loadItemData()
{
    if (m_pItem == nullptr) return;

    QString name = (*m_pItem)["name"].toString("");
    QString description = (*m_pItem)["description"].toString("");
    QString dataType = (*m_pItem)["type"].toString("UNDEFINED");
    int nrOfBytes = (*m_pItem)["bytes"].toInt(0);
    int repeatIndex = (*m_pItem)["repeat_index"].toInt(-1);
    int repeatCount = (*m_pItem)["repeat_count"].toInt(1);

    ui->txtName->setText(name);
    ui->txtDescription->setText(description);
    int index = ui->cmbDataType->findText(dataType);
    if ( index != -1 ) ui->cmbDataType->setCurrentIndex(index);
    ui->spnNrOfBytes->setValue(nrOfBytes);
    ui->spnRepeatIndex->setValue(repeatIndex);
    ui->spnRepeatCount->setValue(repeatCount);
}

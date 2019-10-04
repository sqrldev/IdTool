#include "itemeditordialog.h"
#include "ui_itemeditordialog.h"

ItemEditorDialog::ItemEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ItemEditorDialog)
{
    ui->setupUi(this);
    loadDefaults();

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onSaveButtonClicked()));
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

void ItemEditorDialog::onDataTypeChanged(int currentIndex)
{
    QString sDataType = ui->cmbDataType->itemText(currentIndex);
    ItemDataType idt = IdentityBlockItem::findDataType(sDataType);
    ItemDataTypeInfo idti = IdentityBlockItem::DataTypeMap[idt];

    if (idti.nrOfBytes > 0)
    {
        ui->spnNrOfBytes->setValue(idti.nrOfBytes);
        ui->spnNrOfBytes->setEnabled(false);
    }
    else
    {
        ui->spnNrOfBytes->setValue(0);
        ui->spnNrOfBytes->setEnabled(true);
    }
}

void ItemEditorDialog::onSaveButtonClicked()
{
    if (m_pItem == nullptr) m_pItem = new QJsonObject();

    (*m_pItem)["name"] = ui->txtName->text();
    (*m_pItem)["description"] = ui->txtDescription->text();
    (*m_pItem)["type"] = ui->cmbDataType->currentText();
    (*m_pItem)["bytes"] = ui->spnNrOfBytes->value();

    if (ui->spnRepeatIndex->value() != -1)
    {
        (*m_pItem)["repeat_index"] = ui->spnRepeatIndex->value();
    }

    if (ui->spnRepeatCount->value() > 1)
    {
        (*m_pItem)["repeat_count"] = ui->spnRepeatCount->value();
    }
}

void ItemEditorDialog::loadDefaults()
{
    for (auto const& dataTypeItem : IdentityBlockItem::DataTypeMap)
    {
        ui->cmbDataType->addItem(dataTypeItem.second.name);
    }

    connect(ui->cmbDataType, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataTypeChanged(int)));
    ui->cmbDataType->currentIndexChanged(0);
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

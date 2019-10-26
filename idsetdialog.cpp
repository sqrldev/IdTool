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

#include "idsetdialog.h"
#include "ui_idsetdialog.h"

IdentitySettingsDialog::IdentitySettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IdentitySettingsDialog)
{
    ui->setupUi(this);
    loadDefaults();

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onSaveButtonClicked()));
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(onResetButtonClicked()));
}

IdentitySettingsDialog::IdentitySettingsDialog(QWidget *parent, QJsonObject *item) :
    IdentitySettingsDialog(parent)
{
    m_pItem = item;

    loadDefaults();
    if (m_pItem != nullptr) loadItemData();
}

IdentitySettingsDialog::~IdentitySettingsDialog()
{
    delete ui;
}

QJsonObject *IdentitySettingsDialog::getItem()
{
    return m_pItem;
}

void IdentitySettingsDialog::onDataTypeChanged(int currentIndex)
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

void IdentitySettingsDialog::onSaveButtonClicked()
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

void IdentitySettingsDialog::onResetButtonClicked()
{
    if (m_pItem != nullptr)
    {
        loadItemData();
    }
    else
    {
        ui->txtName->setText("");
        ui->txtDescription->setText("");
        ui->cmbDataType->setCurrentIndex(0);
        ui->spnRepeatIndex->setValue(-1);
        ui->spnRepeatCount->setValue(1);
    }
}

void IdentitySettingsDialog::loadDefaults()
{
    for (auto const& dataTypeItem : IdentityBlockItem::DataTypeMap)
    {
        ui->cmbDataType->addItem(dataTypeItem.second.name);
    }

    connect(ui->cmbDataType, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataTypeChanged(int)));
    ui->cmbDataType->currentIndexChanged(0);
}

void IdentitySettingsDialog::loadItemData()
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

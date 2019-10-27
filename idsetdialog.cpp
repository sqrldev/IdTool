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

void IdentitySettingsDialog::onSaveButtonClicked()
{

}

void IdentitySettingsDialog::onResetButtonClicked()
{

}

void IdentitySettingsDialog::loadDefaults()
{

}

void IdentitySettingsDialog::loadItemData()
{

}

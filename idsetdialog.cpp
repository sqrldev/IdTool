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

IdentitySettingsDialog::IdentitySettingsDialog(QWidget *parent, IdentityBlock* block1) :
    IdentitySettingsDialog(parent)
{
    m_pBlock1 = block1;

    loadDefaults();
    if (m_pBlock1 != nullptr) loadBlockData();
}

IdentitySettingsDialog::~IdentitySettingsDialog()
{
    delete ui;
}

IdentityBlock* IdentitySettingsDialog::getItem()
{
    return m_pBlock1;
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

void IdentitySettingsDialog::loadBlockData()
{
    bool ok = false;
    int optiosnFlags = m_pBlock1->items[7].value.toInt(&ok);
    if (!ok) return;

    ui->chkCheckForUpdates->setChecked(optiosnFlags & 0x0001);
    ui->chkUpdateAutonomously->setChecked((optiosnFlags & 0x0002) >> 1);
    ui->chkRequestSqrlOnlyLogin->setChecked((optiosnFlags & 0x0004) >> 2);
    ui->chkRequstNoSqrlBypass->setChecked((optiosnFlags & 0x0008) >> 3);
    ui->chkMitmWarning->setChecked((optiosnFlags & 0x0010) >> 4);
    ui->chkClearQuickPassOnSleep->setChecked((optiosnFlags & 0x0020) >> 5);
    ui->chkClearQuickPassOnUserSwitch->setChecked((optiosnFlags & 0x0040) >> 6);
    ui->chkEnableQuickPassTimeout->setChecked((optiosnFlags & 0x0080) >> 7);
    ui->chkWarnOnNonCps->setChecked((optiosnFlags & 0x0100) >> 8);

    int quickPassLength = m_pBlock1->items[8].value.toInt(&ok);
    if (!ok) return;
    ui->spnQuickPassLength->setValue(quickPassLength);

    int passwordVerifySecs = m_pBlock1->items[9].value.toInt(&ok);
    if (!ok) return;
    ui->spnPasswordVerifySeconds->setValue(passwordVerifySecs);

    int quickPassTimeout = m_pBlock1->items[10].value.toInt(&ok);
    if (!ok) return;
    ui->spnQuickPassTimeout->setValue(quickPassTimeout);
}

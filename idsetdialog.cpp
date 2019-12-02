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

/*!
 *
 * \class IdentitySettingsDialog
 * \brief A UI dialog for editing user-defined settings of a SQRL identity.
 *
 * Upon saving any changed settings, the user will be asked for the identity
 * password, which is being used to decrypt the identity's type 1 block,
 * obtaining the block's identity keys (IMK and ILK). Consequently,
 * the keys will be reencrypted using the (changed) settings as "additional
 * data" for the authenticated re-encryption of the identity keys.
 *
*/

IdentitySettingsDialog::IdentitySettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IdentitySettingsDialog)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onSaveButtonClicked()));
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(onResetButtonClicked()));
}

IdentitySettingsDialog::IdentitySettingsDialog(QWidget *parent, IdentityBlock* block1) :
    IdentitySettingsDialog(parent)
{
    m_pBlock1 = block1;
    if (m_pBlock1 != nullptr) loadBlockData();
}

IdentitySettingsDialog::~IdentitySettingsDialog()
{
    delete ui;
}

bool IdentitySettingsDialog::hasChanges()
{
    int optionFlags = m_pBlock1->items[7].value.toInt();
    int newOptionFlags = createOptionFlagsInt();
    if (optionFlags != newOptionFlags) return true;

    int quickPassLength = m_pBlock1->items[8].value.toInt();
    if (ui->spnQuickPassLength->value() != quickPassLength) return true;

    int passwordVerifySecs = m_pBlock1->items[9].value.toInt();
    if (ui->spnPasswordVerifySeconds->value() != passwordVerifySecs) return true;

    int quickPassTimeout = m_pBlock1->items[10].value.toInt();
    if (ui->spnQuickPassTimeout->value() != quickPassTimeout) return true;

    return false;
}

int IdentitySettingsDialog::createOptionFlagsInt()
{
    int optionFlags = 0;
    if (ui->chkCheckForUpdates->isChecked()) optionFlags |= 1;
    if (ui->chkUpdateAutonomously->isChecked()) optionFlags |= 0x0002;
    if (ui->chkRequestSqrlOnlyLogin->isChecked()) optionFlags |= 0x0004;
    if (ui->chkRequstNoSqrlBypass->isChecked()) optionFlags |= 0x0008;
    if (ui->chkMitmWarning->isChecked()) optionFlags |= 0x0010;
    if (ui->chkClearQuickPassOnSleep->isChecked()) optionFlags |= 0x0020;
    if (ui->chkClearQuickPassOnUserSwitch->isChecked()) optionFlags |= 0x0040;
    if (ui->chkEnableQuickPassTimeout->isChecked()) optionFlags |= 0x0080;
    if (ui->chkWarnOnNonCps->isChecked()) optionFlags |= 0x0100;

    return optionFlags;
}

void IdentitySettingsDialog::onSaveButtonClicked()
{
    IdentityBlock newBlock1(*m_pBlock1);
    QMessageBox msg(this);

    if (!hasChanges()) return;

    newBlock1.items[7].value = QString::number(createOptionFlagsInt());
    newBlock1.items[8].value = QString::number(ui->spnQuickPassLength->value());
    newBlock1.items[9].value = QString::number(ui->spnPasswordVerifySeconds->value());
    newBlock1.items[10].value = QString::number(ui->spnQuickPassTimeout->value());

    bool ok = false;
    QString password = QInputDialog::getText(
                nullptr,
                tr(""),
                tr("Identity password:"),
                QLineEdit::Password,
                "",
                &ok);

    if (!ok) return;

    QProgressDialog progressDialog(tr("Decrypting identity keys..."), tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    QByteArray key = CryptUtil::createKeyFromPassword(m_pBlock1, password, &progressDialog);
    ok = CryptUtil::updateBlock1(m_pBlock1, &newBlock1, key);

    if (!ok)
    {
        QMessageBox msgBox(this);
        msgBox.critical(this, tr("Error"), tr("Reencryption failed! Wrong password?"));
    }

    *m_pBlock1 = newBlock1;
}

void IdentitySettingsDialog::onResetButtonClicked()
{
    loadBlockData();
}

void IdentitySettingsDialog::loadBlockData()
{
    int optionFlags = m_pBlock1->items[7].value.toInt();

    ui->chkCheckForUpdates->setChecked(optionFlags & 0x0001);
    ui->chkUpdateAutonomously->setChecked((optionFlags & 0x0002) >> 1);
    ui->chkRequestSqrlOnlyLogin->setChecked((optionFlags & 0x0004) >> 2);
    ui->chkRequstNoSqrlBypass->setChecked((optionFlags & 0x0008) >> 3);
    ui->chkMitmWarning->setChecked((optionFlags & 0x0010) >> 4);
    ui->chkClearQuickPassOnSleep->setChecked((optionFlags & 0x0020) >> 5);
    ui->chkClearQuickPassOnUserSwitch->setChecked((optionFlags & 0x0040) >> 6);
    ui->chkEnableQuickPassTimeout->setChecked((optionFlags & 0x0080) >> 7);
    ui->chkWarnOnNonCps->setChecked((optionFlags & 0x0100) >> 8);

    int quickPassLength = m_pBlock1->items[8].value.toInt();
    ui->spnQuickPassLength->setValue(quickPassLength);

    int passwordVerifySecs = m_pBlock1->items[9].value.toInt();
    ui->spnPasswordVerifySeconds->setValue(passwordVerifySecs);

    int quickPassTimeout = m_pBlock1->items[10].value.toInt();
    ui->spnQuickPassTimeout->setValue(quickPassTimeout);
}

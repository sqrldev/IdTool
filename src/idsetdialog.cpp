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
#include "mainwindow.h"

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
 *
*/


/*!
 * Creates a new \c IdentitySettingsDialog window, using \a parent as the
 * parent form and \a identity as the identity who's settings should be changed.
 */

IdentitySettingsDialog::IdentitySettingsDialog(QWidget *parent, IdentityModel* identity) :
    QDialog(parent),
    ui(new Ui::IdentitySettingsDialog)
{
    ui->setupUi(this);
    m_pBlock1 = identity->getBlock(1);
    m_pBlock2 = identity->getBlock(2);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onSaveButtonClicked()));
    connect(ui->buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(onResetButtonClicked()));

    if (m_pBlock1 != nullptr) loadBlockData();
}

/*!
 * Destructor, frees allocated resources.
 */

IdentitySettingsDialog::~IdentitySettingsDialog()
{
    delete ui;
}

/*!
 * Loads the identity settings from the type 1 block provided in the
 * constructor and and pre-fills the UI controls according to the
 * settings values.
 */

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

/*!
 * Determines whether the identity settings were changed in the UI,
 * by comparing the values in the UI controls to those in the stored
 * identity block of type 1.
 *
 * Returns \c true if any of the settings were changed, and \c false
 * otherwise.
 */

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

/*!
 * Checks the values of the checkboxes representing the SQRL identity's
 * "option flags", and returns an integer representation of those bitflags.
 */

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


/***************************************************
 *                S L O T S                        *
 * ************************************************/


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

    QProgressDialog progressDialog("", tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    ok = CryptUtil::updateBlock1WithPassword(m_pBlock1, &newBlock1, password, password, &progressDialog);

    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"), tr("Operation failed! Wrong password?"));
        return;
    }

    *m_pBlock1 = newBlock1;
    
    // Should block 2 be updated as well?
    if (!ui->chk_UpdateBlock2->isChecked()) return;
    
    IdentityBlock newBlock2(*m_pBlock2);
    
    QString rescueCode;
    ok = MainWindow::showGetRescueCodeDialog(rescueCode, this);
    if (!ok) return;
    
    QByteArray decryptedIuk(32, 0);
    ok = CryptUtil::decryptBlock2(decryptedIuk, &newBlock2, rescueCode, &progressDialog);
    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"), tr("Error decrypting block 2! Wrong rescue code?"));
        return;
    }
    
    // Set the same scrypt parameters than on block 1
    newBlock2.items[3].value = newBlock1.items[5].value;
    newBlock2.items[4].value = newBlock1.items[6].value;
    
    // Update block 2
    ok = CryptUtil::updateBlock2(&newBlock2, decryptedIuk, rescueCode, -1, &progressDialog);
    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"), tr("Error updating block 2!"));
        return;
    }
    
    *m_pBlock2 = newBlock2;
}

void IdentitySettingsDialog::onResetButtonClicked()
{
    loadBlockData();   
}

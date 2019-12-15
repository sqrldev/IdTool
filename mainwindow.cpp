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

#include "mainwindow.h"
#include "identityparser.h"
#include "ui_mainwindow.h"
#include "cryptutil.h"
#include "tabmanager.h"
#include <QFileDialog>
#include <QStandardPaths>

/*!
 *
 * \class MainWindow
 * \brief The app's main application window.
 *
 * \c MainWindow provides access to all the functionality offered
 * by tge application through the GUI. It also houses the main
 * application menu.
 *
 * The visual representation of a SQRL identity, represented by an
 * instance of the \c IdentityModel class, is being rendered within
 * the \MainWindow using the \c UiBuilder class.
 *
 * \sa UiBuilder, IdentityModel
 *
*/


/*!
 * Creates a new \c MainWindow, using \a parent as the parent form.
 */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSize desktopSize = QDesktopWidget().availableGeometry(this).size() * 0.9;
    resize(geometry().width(), desktopSize.height());

    m_pTabManager = new TabManager(ui->tabWidget);
    onControlUnauthenticatedChanges();

    connect(ui->actionCreateNewIdentity, &QAction::triggered, this, &MainWindow::onCreateNewIdentity);
    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(ui->actionSaveIdentityFileAs, &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onShowAboutDialog);
    connect(ui->actionPasteIdentityData, &QAction::triggered, this, &MainWindow::onPasteIdentityText);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onQuit);
    connect(ui->actionBuildNewIdentity, &QAction::triggered, this, &MainWindow::onBuildNewIdentity);
    connect(ui->actionBlockDesigner, &QAction::triggered, this, &MainWindow::onShowBlockDesigner);
    connect(ui->actionDecryptImkIlk, &QAction::triggered, this, &MainWindow::onDecryptImkIlk);
    connect(ui->actionDecryptIuk, &QAction::triggered, this, &MainWindow::onDecryptIuk);
    connect(ui->actionDecryptPreviousIuks, &QAction::triggered, this, &MainWindow::onDecryptPreviousIuks);
    connect(ui->actionCreateSiteKeys, &QAction::triggered, this, &MainWindow::onCreateSiteKeys);
    connect(ui->actionIdentitySettings, &QAction::triggered, this, &MainWindow::onShowIdentitySettingsDialog);
    connect(ui->actionEnableUnauthenticatedChanges, &QAction::triggered, this, &MainWindow::onControlUnauthenticatedChanges);
    connect(ui->actionDisplayTextualIdentity, &QAction::triggered, this, &MainWindow::onDisplayTextualIdentity);
    connect(ui->actionImportTextualIdentity, &QAction::triggered, this, &MainWindow::onImportTextualIdentity);
    connect(ui->actionChangePassword, &QAction::triggered, this, &MainWindow::onChangePassword);
    connect(ui->actionResetPassword, &QAction::triggered, this, &MainWindow::onResetPassword);
    connect(m_pTabManager, &TabManager::currentTabChanged, this, &MainWindow::onCurrentTabChanged);

    configureMenuItems();
}

/*!
 * Destructor, frees allocated resources.
 */

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pTabManager;
}

/*!
 * Displays an error message-box, telling the user that an identity
 * needs to be loaded to complete the operation.
 */

void MainWindow::showNoIdentityLoadedError()
{
    QMessageBox::critical(this, tr("Error"), tr("An identity needs to be loaded "
                                                "in order to complete this operation!"));
}

/*!
 * Displays a dialog window containing the currently loaded itentity's textual
 * representation along with some descriptive information.
 *
 * If \a rescueCode is provided, it will also be shown right before the textual
 * identity.
 */

void MainWindow::showTextualIdentityInfoDialog(QString rescueCode)
{
    QString messageText = "!!!!" + tr("RECORD THIS INFORMATION AND STORE IT SAFELY") +
            "!!!!\r\n\r\n" ;

    if (rescueCode != nullptr && rescueCode != "")
    {
        messageText += tr("Rescue code:") + " " + CryptUtil::formatRescueCode(rescueCode);
        messageText += "\r\n\r\n";
    }
    messageText += tr("Textual version of the identity:");
    messageText += "\r\n\r\n";
    messageText += m_pTabManager->getCurrentTab()
            .getIdentityModel().getTextualVersionFormatted();

    QInputDialog resultDialog(this);
    resultDialog.setInputMode(QInputDialog::TextInput);
    resultDialog.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    resultDialog.resize(700, 400);
    resultDialog.setWindowTitle(tr("Success"));
    resultDialog.setLabelText(tr("The identity was successfully created!"));
    resultDialog.setTextValue(messageText);
    resultDialog.exec();
}

/*!
 * Checks if at least one identity is loaded and enables or
 * disables main menu entries / actions accrodingly.
 */

void MainWindow::configureMenuItems()
{
    int currentIndex = m_pTabManager->getCurrentTabIndex();
    bool enable = currentIndex != -1 ? true : false;
    bool enableBlock3Ops = false;

    if (currentIndex != -1)
    {
        if (m_pTabManager->getCurrentTab()
                .getIdentityModel().hasBlockType(3))
        {
            enableBlock3Ops = true;
        }
    }

    ui->actionDecryptIuk->setEnabled(enable);
    ui->actionDecryptImkIlk->setEnabled(enable);
    ui->actionChangePassword->setEnabled(enable);
    ui->actionResetPassword->setEnabled(enable);
    ui->actionCreateSiteKeys->setEnabled(enable);
    ui->actionIdentitySettings->setEnabled(enable);
    ui->actionSaveIdentityFileAs->setEnabled(enable);
    ui->actionDecryptPreviousIuks->setEnabled(enable && enableBlock3Ops);
    ui->actionDisplayTextualIdentity->setEnabled(enable);
}

/*!
 * Displays a dialog, setting \a parent as the dialog's parent widget,
 * prompting the user to input the current identity's rescue code.
 * The string being input is then stripped of all dashes ("-") and spaces (" ").
 *
 * If the dialog was not cancelled and the string is not empty after stripping
 * it of unwanted characters, \c true is returned and the input string is
 * placed into \a rescueCode. Otherwise, \c false is returned.
 */

bool MainWindow::showGetRescueCodeDialog(QString& rescueCode, QWidget* parent)
{
    bool ok = false;

    QString rc = QInputDialog::getText(
                parent,
                tr(""),
                tr("Identity rescue code:"),
                QLineEdit::Normal,
                "",
                &ok);

    if (!ok) return false;

    rc = rc.replace("-", "");
    rc = rc.replace(" ", "");

    if (rc == "") return false;

    rescueCode = rc;
    return true;
}

/*!
 * Displays a dialog, prompting the user to input the identity's master
 * password.
 *
 * If the dialog was not cancelled, \c true is returned and the input
 * string is placed into \a password. Otherwise, \c false is returned.
 */

bool MainWindow::showGetPasswordDialog(QString &password, QWidget* parent)
{
    bool ok = false;
    QString pwTemp = QInputDialog::getText(
                parent,
                tr(""),
                tr("Please enter your current identity master password:"),
                QLineEdit::Password,
                "",
                &ok);

    if (ok) password = pwTemp;
    return ok;
}

/*!
 * Displays a dialog, prompting the user to input and confirm a new identity
 * master password.
 *
 * If the dialog was not cancelled and if the password and the confirmation
 * match up, \c true is returned and the input string is placed into
 * \a password. Otherwise, \c false is returned.
 */

bool MainWindow::showGetNewPasswordDialog(QString &password, QWidget* parent)
{
    bool ok = false;
    QString password1, password2;

    password1 = QInputDialog::getText(
                parent,
                tr(""),
                tr("Please enter the new identity master password:"),
                QLineEdit::Password,
                "",
                &ok);

    if (!ok) return false;

    do
    {
        password2 = QInputDialog::getText(
                parent,
                tr(""),
                tr("Please confirm the new password:"),
                QLineEdit::Password,
                "",
                &ok);

        if (!ok) return false;
    }
    while (password1 != password2);

    password = password1;
    return true;
}

/*!
 * Displays a dialog, asking the user to confirm that any changes that were
 * made to any of the loaded identities can be discarded.
 *
 * Returns \c true if the user confirmed the message, or \c false otherwise.
 */

bool MainWindow::canDiscardChanges()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(m_pTabWidget, tr("Unsaved changes"),
        tr("You have unsaved changes! Do you really want to close the "
           "application without saving the changes?"));

    if (reply == QMessageBox::No)  return false;
    else return true;
}


/***************************************************
 *              O V E R R I D E S                  *
 * ************************************************/


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_pTabManager->hasDirtyTabs())
    {
        if (!canDiscardChanges()) event->ignore();
        return;
    }

    event->accept();
}


/***************************************************
 *                S L O T S                        *
 * ************************************************/


void MainWindow::onCreateNewIdentity()
{
    IdentityModel* pIidentity = new IdentityModel();
    QString password;
    QString rescueCode;
    bool ok = false;

    ok = showGetNewPasswordDialog(password);
    if (!ok) return;

    QProgressDialog progressDialog(tr("Generating and encrypting identity..."),
                                   tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    ok = CryptUtil::createIdentity(*pIidentity, rescueCode, password,
                                   &progressDialog);
    progressDialog.close();

    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("An error occured while creating the identity "
               "or the operation was aborted by the user."));
        return;
    }

    m_pTabManager->addTab(*pIidentity, QFileInfo());
    m_pTabManager->setCurrentTabDirty(true);

    showTextualIdentityInfoDialog(rescueCode);

    onSaveFile();
}

void MainWindow::onDisplayTextualIdentity()
{
    showTextualIdentityInfoDialog();
}

void MainWindow::onImportTextualIdentity()
{
    bool ok = false;
    QString textualIdentity = QInputDialog::getMultiLineText(
                this,
                tr("Import textual identity data"),
                tr("Type or paste textual identity data here:"),
                "",
                &ok);

    if (!ok) return;

    try
    {
        QByteArray identityBytes = CryptUtil::base56DecodeIdentity(textualIdentity);

        if (identityBytes == nullptr)
        {
            QMessageBox::critical(this, tr("Error"), tr("Invalid identity data!"));
            return;
        }

        identityBytes = IdentityParser::HEADER.toLocal8Bit() + identityBytes;

        IdentityModel* pIdentity = new IdentityModel();
        IdentityParser parser;
        parser.parseIdentityData(identityBytes, pIdentity);

        IdentityBlock* pBlock2 = pIdentity->getBlock(2);

        QString rescueCode;
        ok = showGetRescueCodeDialog(rescueCode);
        if (!ok) return;

        QString password;
        ok = showGetNewPasswordDialog(password);
        if (!ok) return;

        QByteArray decryptedIuk(32, 0);
        QProgressDialog progressDialog(tr("Decrypting IUK..."),
                                       tr("Abort"), 0, 0, this);
        progressDialog.setWindowModality(Qt::WindowModal);
        CryptUtil::decryptBlock2(decryptedIuk, pBlock2, rescueCode, &progressDialog);

        progressDialog.setLabelText(tr("Encrypting IMK and ILK..."));
        IdentityBlock block1 = CryptUtil::createBlock1(decryptedIuk, password, &progressDialog);

        pIdentity->blocks.insert(pIdentity->blocks.begin(), block1);

        m_pTabManager->addTab(*pIdentity, QFileInfo());
        m_pTabManager->setCurrentTabDirty(true);
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
}

void MainWindow::onOpenFile()
{
    QString dir = nullptr;

    const QStringList dirs = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation);
    if (dirs.count() > 0)
    {
        dir = QDir(dirs[0]).filePath("SQRL/");
    }

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open identity file"), dir, tr("SQRL identity files (*.sqrl *.sqrc)"));

    if (fileName.isEmpty()) return;

    try
    {
        IdentityModel* pModel = new IdentityModel();
        IdentityParser parser;
        parser.parseFile(fileName, pModel);
        m_pTabManager->addTab(*pModel, QFileInfo(fileName));
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
}

void MainWindow::onSaveFile()
{
    QString dir = nullptr;

    const QStringList dirs = QStandardPaths::standardLocations(
                QStandardPaths::DocumentsLocation);
    if (dirs.count() > 0)
    {
        dir = QDir(dirs[0]).filePath("SQRL/");
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save identity file"), dir, tr("SQRL identity files (*.sqrl *.sqrc)"));

    if (fileName.isEmpty()) return;

    try
    {
        m_pTabManager->getCurrentTab().getIdentityModel()
                .writeToFile(fileName);
        QFileInfo fileInfo(fileName);
        m_pTabManager->getCurrentTab().updateFileInfo(fileInfo);
        m_pTabManager->setCurrentTabDirty(false);
        m_pTabManager->updateCurrentTabText();
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
}

void MainWindow::onShowAboutDialog()
{
    QString message = "<b>IdTool</b><br>";
    message.append(tr("Version") + ": ");
    message.append(QString(APP_VERSION) + "<br><br>");
    message.append(tr("Author") + ": ");
    message.append("Alex Hauser<br><br>");
    message.append(tr("IdTool is open source software") + ":<br>");
    message.append("<a href=\"https://github.com/sqrldev/IdTool\">"
                   "https://github.com/sqrldev/IdTool</a>");

    QMessageBox::about(this, "About", message);
}

void MainWindow::onChangePassword()
{
    QString password;
    bool ok = showGetPasswordDialog(password, this);
    if (!ok) return;

    QString newPassword;
    ok = showGetNewPasswordDialog(newPassword);
    if (!ok) return;

    IdentityBlock* block1 = m_pTabManager->getCurrentTab()
            .getIdentityModel().getBlock(1);
    if (block1 == nullptr) return;
    IdentityBlock newBlock1 = *block1;

    QProgressDialog progressDialog("", tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    ok = CryptUtil::updateBlock1WithPassword(block1, &newBlock1, password, newPassword, &progressDialog);
    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Error updating password! Wrong current password?"));
        return;
    }

    *block1 = newBlock1;
    m_pTabManager->getCurrentTab().rebuild();
    m_pTabManager->setCurrentTabDirty(true);
}

void MainWindow::onResetPassword()
{
    QString rescueCode;
    bool ok = showGetRescueCodeDialog(rescueCode, this);
    if (!ok) return;

    QString password;
    ok = showGetNewPasswordDialog(password, this);
    if (!ok) return;

    IdentityBlock* pBlock2 = m_pTabManager->getCurrentTab()
            .getIdentityModel().getBlock(2);
    if (pBlock2 == nullptr) return;

    QProgressDialog progressDialog(tr("Decrypting block 2..."),
                                   tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    QByteArray decryptedIuk(32, 0);
    ok = CryptUtil::decryptBlock2(decryptedIuk, pBlock2, rescueCode, &progressDialog);
    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Error decrypting block2!"));
        return;
    }

    QByteArray decryptedImk = CryptUtil::createImkFromIuk(decryptedIuk);
    QByteArray decryptedIlk = CryptUtil::createIlkFromIuk(decryptedIuk);


    IdentityBlock* pBlock1 = m_pTabManager->getCurrentTab()
            .getIdentityModel().getBlock(1);
    if (pBlock1 == nullptr) return;

    IdentityBlock updatedBlock1 = *pBlock1;

    ok = CryptUtil::updateBlock1(&updatedBlock1, decryptedImk, decryptedIlk,
                                 password, &progressDialog);
    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Error updating block type 1!"));
        return;
    }

    *pBlock1 = updatedBlock1;

    m_pTabManager->getCurrentTab().rebuild();
    m_pTabManager->setCurrentTabDirty(true);

    QMessageBox::information(this, tr("Success"),
        tr("Password has been successfully reset!"));
}

void MainWindow::onShowIdentitySettingsDialog()
{
    IdentityBlock* pBlock1 = m_pTabManager->getCurrentTab()
            .getIdentityModel().getBlock(1);
    if (pBlock1 == nullptr) return;

    IdentitySettingsDialog dialog(this, pBlock1);
    if (dialog.exec() == QDialog::Accepted)
    {
        m_pTabManager->getCurrentTab().rebuild();
        m_pTabManager->setCurrentTabDirty(true);
    }
}

void MainWindow::onControlUnauthenticatedChanges()
{
    bool enable = ui->actionEnableUnauthenticatedChanges->isChecked();

    if (enable)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    this,
                    tr("Enabling unauthenticated changes"),
                    tr("CAUTION!\n\nEnabling unauthenticated changes will break the validity "
                       "of the identity! While this feature may be useful for developers, "
                       "making unauthenticated changes WILL break the identity's cryptographic "
                       "authentication and therefore make it usesless for all standard "
                       "operations!\n\nAre you sure you want to continue?"),
                    QMessageBox::Yes|QMessageBox::No);

        if (reply == QMessageBox::No)
        {
            ui->actionEnableUnauthenticatedChanges->setChecked(false);
            m_pTabManager->setEnableUnauthenticatedChanges(false);
            return;
        }
    }

    ui->actionBuildNewIdentity->setVisible(enable);
    m_pTabManager->setEnableUnauthenticatedChanges(enable);
}

void MainWindow::onPasteIdentityText()
{
    bool ok = false;
    QString result = QInputDialog::getMultiLineText(
                this,
                tr("Paste identity data"),
                tr("Paste base64-encoded identity data here:"),
                "",
                &ok);

    if (!ok) return;

    if (result.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Invalid identity data!"));
    }

    try
    {
        IdentityParser parser;
        IdentityModel* pModel = new IdentityModel();
        parser.parseString(result, pModel);
        m_pTabManager->addTab(*pModel, QFileInfo());
        m_pTabManager->setCurrentTabDirty(true);
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
}

void MainWindow::onBuildNewIdentity()
{
    QString sBlockType;

    bool ok = UiBuilder::showGetBlockTypeDialog(&sBlockType);
    if (!ok) return;

    ushort blockType = sBlockType.toUShort(&ok);
    if (!ok) return;

    IdentityBlock block = IdentityParser::
            createEmptyBlock(blockType);


    IdentityModel* pModel = new IdentityModel();
    pModel->blocks.push_back(block);
    m_pTabManager->addTab(*pModel, QFileInfo());
    m_pTabManager->setCurrentTabDirty(true);
}

void MainWindow::onShowBlockDesigner()
{
    bool ok = false;
    QString sBlockType;

    if (!UiBuilder::showGetBlockTypeDialog(&sBlockType, true))
        return;

    int iBlockType = sBlockType.toInt(&ok);

    if (!ok) return;

    BlockDesignerDialog dialog(iBlockType, this);
    dialog.exec();
}

void MainWindow::onCreateSiteKeys()
{
    IdentityBlock* pBlock = m_pTabManager->getCurrentTab()
            .getIdentityModel().getBlock(1);
    if (pBlock == nullptr) return;

    bool ok = false;

    QString domain = QInputDialog::getText(
                nullptr,
                tr(""),
                tr("Domain:"),
                QLineEdit::Normal,
                "",
                &ok);

    if (!ok) return;

    QString password;
    ok = showGetPasswordDialog(password, this);
    if (!ok) return;

    QProgressDialog progressDialog(tr("Decrypting identity keys..."),
                                   tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    QByteArray key;
    ok = CryptUtil::createKeyFromPassword(
                key,
                *pBlock,
                password,
                &progressDialog);
    progressDialog.close();
    if (!ok) return;

    QByteArray decryptedImk(32, 0);
    QByteArray decryptedIlk(32, 0);

    if (!CryptUtil::decryptBlock1(
                decryptedImk,
                decryptedIlk,
                pBlock,
                key))
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Decryption of identity keys failed! Wrong password?"));
        return;
    }

    QByteArray publicKey(crypto_sign_PUBLICKEYBYTES, 0);
    QByteArray privateKey(crypto_sign_SECRETKEYBYTES, 0);

    if ( !CryptUtil::createSiteKeys(
                publicKey,
                privateKey,
                domain,
                decryptedImk))
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Creation of site keys failed, probably due"
               "to an error in the crypto routines!"));
        return;
    }

    QByteArray result("Site-specific private key:\n");
    result.append(privateKey.toHex());
    result.append("\n\nSite-specific public key:\n");
    result.append(publicKey.toHex());

    QInputDialog resultDialog(this);
    resultDialog.setInputMode(QInputDialog::TextInput);
    resultDialog.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    resultDialog.resize(700, 250);
    resultDialog.setWindowTitle("Success");
    resultDialog.setLabelText("Creation of site keys succeeded!");
    resultDialog.setTextValue(result);
    resultDialog.exec();
}

void MainWindow::onDecryptImkIlk()
{
    IdentityBlock* pBlock1 = m_pTabManager->getCurrentTab()
            .getIdentityModel().getBlock(1);
    if (pBlock1 == nullptr)
    {
        QMessageBox::critical(this, tr("Error"),
                        tr("The loaded identity does not have a type 1 block!"));
        return;
    }

    bool ok = false;

    QString password;
    ok = showGetPasswordDialog(password, this);
    if (!ok) return;

    QProgressDialog progressDialog(tr("Decrypting identity keys..."),
                                   tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    QByteArray key;
    ok = CryptUtil::createKeyFromPassword(
                key,
                *pBlock1,
                password,
                &progressDialog);
    progressDialog.close();
    if (!ok) return;

    QByteArray decryptedImk(32, 0);
    QByteArray decryptedIlk(32, 0);

    if (!CryptUtil::decryptBlock1(
                decryptedImk,
                decryptedIlk,
                pBlock1,
                key))
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Decryption of identity keys failed! Wrong password?"));
        return;
    }

    QByteArray result("Identity Master Key:\n");
    result.append(decryptedImk.toHex());
    result.append("\n\nIdentity Lock Key:\n");
    result.append(decryptedIlk.toHex());

    QInputDialog resultDialog(this);
    resultDialog.setInputMode(QInputDialog::TextInput);
    resultDialog.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    resultDialog.resize(700, 250);
    resultDialog.setWindowTitle("Success");
    resultDialog.setLabelText("Decryption of identitiy keys succeeded!");
    resultDialog.setTextValue(result);
    resultDialog.exec();
}

void MainWindow::onDecryptIuk()
{
    IdentityBlock* pBlock2 = m_pTabManager->getCurrentTab()
            .getIdentityModel().getBlock(2);
    if (pBlock2 == nullptr)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("The loaded identity does not have a type 2 block!"));
        return;
    }

    QString rescueCode;
    bool ok = showGetRescueCodeDialog(rescueCode, this);
    if (!ok) return;

    QByteArray decryptedIuk(32, 0);

    QProgressDialog progressDialog(tr("Decrypting identity unlock key..."),
                                   tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    ok = CryptUtil::decryptBlock2(
                decryptedIuk,
                pBlock2,
                rescueCode,
                &progressDialog);

    progressDialog.close();

    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Decryption of identity unlock key failed! Wrong password?"));
        return;
    }

    QByteArray result("Identity Unlock Key:\n");
    result.append(decryptedIuk.toHex());

    QInputDialog resultDialog(this);
    resultDialog.setInputMode(QInputDialog::TextInput);
    resultDialog.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    resultDialog.resize(700, 250);
    resultDialog.setWindowTitle("Success");
    resultDialog.setLabelText("Decryption of identitiy unlock key succeeded!");
    resultDialog.setTextValue(result);
    resultDialog.exec();
}

void MainWindow::onDecryptPreviousIuks()
{
    const QString DECRYPTION_METHOD_PASSWORD = tr("Password");
    const QString DECRYPTION_METHOD_RESCUE_CODE = tr("Rescue Code");
    QByteArray decryptedImk(32, 0);
    QByteArray decryptedIlk(32, 0);

    IdentityBlock* pBlock3 = m_pTabManager->getCurrentTab()
            .getIdentityModel().getBlock(3);
    if (pBlock3 == nullptr)
    {
        QMessageBox::critical(this, tr("Error"),
            tr("The loaded identity does not have a type 3 block!"));
        return;
    }

    bool ok = false;

    QString decryptionMethod = QInputDialog::getItem(
                this,
                tr(""),
                tr("Choose decrytion method:"),
                QStringList( { QString(DECRYPTION_METHOD_PASSWORD),
                               QString(DECRYPTION_METHOD_RESCUE_CODE) } ),
                0,
                false,
                &ok);

    if (!ok) return;

    if (decryptionMethod == DECRYPTION_METHOD_PASSWORD)
    {
        IdentityBlock* pBlock1 = m_pTabManager->getCurrentTab()
                .getIdentityModel().getBlock(1);

        if (pBlock1 == nullptr)
        {
            QMessageBox::critical(this, tr("Error"),
                tr("The loaded identity does not have a type 1 block!"));
            return;
        }

        QString password;
        ok = showGetPasswordDialog(password, this);
        if (!ok) return;

        QProgressDialog progressDialog(tr("Decrypting identity keys..."),
                                       tr("Abort"), 0, 0, this);
        progressDialog.setWindowModality(Qt::WindowModal);

        QByteArray key;
        ok = CryptUtil::createKeyFromPassword(
                    key,
                    *pBlock1,
                    password,
                    &progressDialog);
        progressDialog.close();
        if (!ok) return;

        if (!CryptUtil::decryptBlock1(
                    decryptedImk,
                    decryptedIlk,
                    pBlock1,
                    key))
        {
            QMessageBox::critical(this, tr("Error"),
                tr("Decryption of identity keys failed! Wrong password?"));
            return;
        }
    }
    else if (decryptionMethod == DECRYPTION_METHOD_RESCUE_CODE)
    {
        IdentityBlock* pBlock2 = m_pTabManager->getCurrentTab().
                getIdentityModel().getBlock(2);
        if (pBlock2 == nullptr)
        {
            QMessageBox::critical(this, tr("Error"),
                tr("The loaded identity does not have a type 2 block!"));
            return;
        }

        QString rescueCode = QInputDialog::getText(
                    nullptr,
                    tr(""),
                    tr("Identity rescue code:"),
                    QLineEdit::Normal,
                    "",
                    &ok);

        if (!ok) return;

        rescueCode = rescueCode.replace("-", "");
        rescueCode = rescueCode.replace(" ", "");

        QProgressDialog progressDialog(tr("Decrypting identity unlock key..."),
                                       tr("Abort"), 0, 0, this);
        progressDialog.setWindowModality(Qt::WindowModal);

        QByteArray decryptedIuk(32, 0);

        ok = CryptUtil::decryptBlock2(
                    decryptedIuk,
                    pBlock2,
                    rescueCode,
                    &progressDialog);

        progressDialog.close();

        if (!ok)
        {
            QMessageBox::critical(this, tr("Error"),
                tr("Decryption of identity unlock key failed! "
                   "Wrong password?"));
            return;
        }

        decryptedImk = CryptUtil::createImkFromIuk(decryptedIuk);
    }

    QList<QByteArray> previousIuks;

    if (!CryptUtil::decryptBlock3(
                previousIuks,
                pBlock3,
                decryptedImk))
    {
        QMessageBox::critical(this, tr("Error"), tr("Decryption of previous identity keys"
                                                    "failed! Wrong password?"));
        return;
    }

    QByteArray result;
    for (int i=0; i<previousIuks.length(); i++)
    {
        result.append(QString("Identity Unlock Key %1:\n").arg(i+1));
        result.append(previousIuks[i].toHex());
        result.append("\n\n");
    }

    QInputDialog resultDialog(this);
    resultDialog.setInputMode(QInputDialog::TextInput);
    resultDialog.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    resultDialog.resize(700, 250);
    resultDialog.setWindowTitle("Success");
    resultDialog.setLabelText("Decryption of previous identitiy unlock keys succeeded!");
    resultDialog.setTextValue(result);
    resultDialog.exec();

}

void MainWindow::onCurrentTabChanged(int index)
{
    configureMenuItems();
}

void MainWindow::onQuit()
{

    if (m_pTabManager->hasDirtyTabs())
    {
        if (!canDiscardChanges()) return;
    }

    QApplication::quit();
}

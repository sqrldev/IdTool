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
#include <QFileDialog>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSize desktopSize = QDesktopWidget().availableGeometry(this).size() * 0.9;
    resize(geometry().width(), desktopSize.height());

    m_pHeaderFrame = this->findChild<QFrame*>("headerFrame");
    m_pScrollArea = this->findChild<QScrollArea*>("scrollArea");
    m_pIdentityParser = new IdentityParser();
    m_pIdentityModel = new IdentityModel();
    m_pUiBuilder = new UiBuilder(this, m_pIdentityModel);
    m_pHeaderFrame->setVisible(false);

    m_pScrollArea->setWidgetResizable(true);
    controlUnauthenticatedChanges();

    connect(ui->actionCreateNewIdentity, &QAction::triggered, this, &MainWindow::createNewIdentity);
    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSaveIdentityFileAs, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ui->actionPasteIdentityData, &QAction::triggered, this, &MainWindow::pasteIdentityText);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::quit);
    connect(ui->actionBuildNewIdentity, &QAction::triggered, this, &MainWindow::buildNewIdentity);
    connect(ui->actionBlockDesigner, &QAction::triggered, this, &MainWindow::showBlockDesigner);
    connect(ui->actionDecryptImkIlk, &QAction::triggered, this, &MainWindow::decryptImkIlk);
    connect(ui->actionDecryptIuk, &QAction::triggered, this, &MainWindow::decryptIuk);
    connect(ui->actionDecryptPreviousIuks, &QAction::triggered, this, &MainWindow::decryptPreviousIuks);
    connect(ui->actionCreateSiteKeys, &QAction::triggered, this, &MainWindow::createSiteKeys);
    connect(ui->actionIdentitySettings, &QAction::triggered, this, &MainWindow::showIdentitySettingsDialog);
    connect(ui->actionEnableUnauthenticatedChanges, &QAction::triggered, this, &MainWindow::controlUnauthenticatedChanges);
    connect(ui->actionDisplayTextualIdentity, &QAction::triggered, this, &MainWindow::displayTextualIdentity);
    connect(ui->actionImportTextualIdentity, &QAction::triggered, this, &MainWindow::importTextualIdentity);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pIdentityParser;
    delete m_pIdentityModel;
    delete m_pUiBuilder;
}

void MainWindow::showNoIdentityLoadedError()
{
    QMessageBox::critical(this, tr("Error"), tr("An identity needs to be loaded"
                                                "in order to complete this operation!"));
}

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
    messageText += m_pIdentityModel->getTextualVersionFormatted();

    QInputDialog resultDialog(this);
    resultDialog.setInputMode(QInputDialog::TextInput);
    resultDialog.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    resultDialog.resize(700, 400);
    resultDialog.setWindowTitle(tr("Success"));
    resultDialog.setLabelText(tr("The identity was successfully created!"));
    resultDialog.setTextValue(messageText);
    resultDialog.exec();
}

bool MainWindow::showRescueCodeInputDialog(QString& rescueCode)
{
    bool ok = false;

    QString rc = QInputDialog::getText(
                nullptr,
                tr(""),
                tr("Identity rescue code:"),
                QLineEdit::Normal,
                "",
                &ok);

    if (!ok) return false;

    rc = rc.replace("-", "");
    rc = rc.replace(" ", "");

    rescueCode = rc;
    return true;
}

bool MainWindow::showGetNewPasswordDialog(QString &password)
{
    bool ok = false;
    QString password1, password2;

    password1 = QInputDialog::getText(
                nullptr,
                tr(""),
                tr("Set identity master password:"),
                QLineEdit::Password,
                "",
                &ok);

    if (!ok) return false;

    do
    {
        password2 = QInputDialog::getText(
                nullptr,
                tr(""),
                tr("Confirm password:"),
                QLineEdit::Password,
                "",
                &ok);

        if (!ok) return false;
    }
    while (password1 != password2);

    password = password1;
    return true;
}

bool MainWindow::canDiscardCurrentIdentity()
{
    if (m_pUiBuilder->hasBlocks())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("This operation will discard any existing identity information!"));
        msgBox.setInformativeText(tr("Do you really want to continue and discard all unsaved changes?"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);

        if (msgBox.exec() != QMessageBox::Ok) return false;
    }

    return true;
}

void MainWindow::createNewIdentity()
{
    IdentityModel identity;
    QString rescueCode;
    bool ok = false;

    if (!canDiscardCurrentIdentity()) return;

    QString password;
    ok = showGetNewPasswordDialog(password);
    if (!ok) return;

    QProgressDialog progressDialog(tr("Generating and encrypting identity..."),
                                   tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    ok = CryptUtil::createIdentity(identity, rescueCode, password,
                                   &progressDialog);

    progressDialog.close();

    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"),
                              tr("An error occured while creating the identity "
                                 "or the operation was aborted by the user."));
        return;
    }

    m_pIdentityModel->import(identity);
    m_pUiBuilder->rebuild();

    showTextualIdentityInfoDialog(rescueCode);

    saveFile();
}

void MainWindow::displayTextualIdentity()
{
    if (m_pIdentityModel == nullptr ||
        m_pIdentityModel->blocks.size() < 1)
    {
        showNoIdentityLoadedError();
        return;
    }

    showTextualIdentityInfoDialog();
}

void MainWindow::importTextualIdentity()
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

        IdentityModel identity;
        IdentityParser parser;
        parser.parseIdentityData(identityBytes, &identity);

        IdentityBlock* pBlock2 = identity.getBlock(2);

        QString rescueCode;
        ok = showRescueCodeInputDialog(rescueCode);
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

        identity.blocks.insert(identity.blocks.begin(), block1);

        m_pIdentityModel->clear();
        m_pIdentityModel->import(identity);
        m_pUiBuilder->rebuild();
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
}

void MainWindow::openFile()
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

    QByteArray ba = fileName.toLocal8Bit();
    char *pszFileName = ba.data();

    try
    {
        m_pIdentityModel->clear();
        m_pIdentityParser->parseFile(pszFileName, m_pIdentityModel);
        m_pUiBuilder->rebuild();

        //m_pHeaderFrame->setVisible(true);
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
}

void MainWindow::saveFile()
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
        m_pIdentityModel->writeToFile(fileName);
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
}

void MainWindow::showAboutDialog()
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

void MainWindow::showIdentitySettingsDialog()
{
    if (m_pIdentityModel == nullptr ||
        m_pIdentityModel->blocks.size() < 1)
    {
        showNoIdentityLoadedError();
        return;
    }

    IdentityBlock* pBlock1 = m_pIdentityModel->getBlock(1);
    if (pBlock1 == nullptr) return;

    IdentitySettingsDialog dialog(this, pBlock1);
    dialog.exec();

    m_pUiBuilder->rebuild();
}

void MainWindow::controlUnauthenticatedChanges()
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
            return;
        }
    }

    m_pUiBuilder->setEnableUnauthenticatedChanges(
                enable, true);

    ui->actionBuildNewIdentity->setVisible(enable);
}

void MainWindow::pasteIdentityText()
{
    bool ok = false;
    QString result = QInputDialog::getMultiLineText(
                this,
                tr("Paste identity data"),
                tr("Paste base64-encoded identity data here:"),
                "",
                &ok);

    if (!ok) return;

    try
    {
        if (result.isEmpty())
        {
            QMessageBox::critical(this, tr("Error"), tr("Invalid identity data!"));
        }

        m_pIdentityModel->clear();
        m_pIdentityParser->parseText(result, m_pIdentityModel);
        m_pUiBuilder->rebuild();
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
}

void MainWindow::buildNewIdentity()
{
    QString sBlockType;

    if (!canDiscardCurrentIdentity()) return;

    bool ok = UiBuilder::showGetBlockTypeDialog(&sBlockType);
    if (!ok) return;

    ushort blockType = sBlockType.toUShort(&ok);
    if (!ok) return;

    IdentityBlock block = m_pIdentityParser->
            createEmptyBlock(blockType);

    m_pIdentityModel->clear();
    m_pIdentityModel->blocks.push_back(block);
    m_pUiBuilder->rebuild();
}

void MainWindow::showBlockDesigner()
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

void MainWindow::createSiteKeys()
{
    if (m_pIdentityModel == nullptr ||
        m_pIdentityModel->blocks.size() < 1)
    {
        showNoIdentityLoadedError();
        return;
    }

    IdentityBlock* pBlock = m_pIdentityModel->getBlock(1);
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

    QByteArray key = CryptUtil::createKeyFromPassword(
                pBlock,
                password,
                &progressDialog);

    progressDialog.close();

    QByteArray decryptedImk(32, 0);
    QByteArray decryptedIlk(32, 0);

    if (!CryptUtil::decryptBlock1(
                decryptedImk,
                decryptedIlk,
                pBlock,
                key))
    {
        QMessageBox::critical(this, tr("Error"), tr("Decryption of identity keys failed! Wrong password?"));
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
        QMessageBox::critical(this, tr("Error"), tr("Creation of site keys failed,"
                                              "probably due to an error in the crypto routines!"));
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

void MainWindow::decryptImkIlk()
{
    if (m_pIdentityModel == nullptr ||
        m_pIdentityModel->blocks.size() < 1)
    {
        showNoIdentityLoadedError();
        return;
    }

    IdentityBlock* pBlock1 = m_pIdentityModel->getBlock(1);
    if (pBlock1 == nullptr)
    {
        QMessageBox::critical(this, tr("Error"),
                        tr("The loaded identity does not have a type 1 block!"));
        return;
    }

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

    QByteArray key = CryptUtil::createKeyFromPassword(
                pBlock1,
                password,
                &progressDialog);

    progressDialog.close();

    QByteArray decryptedImk(32, 0);
    QByteArray decryptedIlk(32, 0);

    if (!CryptUtil::decryptBlock1(
                decryptedImk,
                decryptedIlk,
                pBlock1,
                key))
    {
        QMessageBox::critical(this, tr("Error"), tr("Decryption of identity keys failed! Wrong password?"));
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

void MainWindow::decryptIuk()
{
    if (m_pIdentityModel == nullptr ||
        m_pIdentityModel->blocks.size() < 1)
    {
        showNoIdentityLoadedError();
        return;
    }

    IdentityBlock* pBlock2 = m_pIdentityModel->getBlock(2);
    if (pBlock2 == nullptr)
    {
        QMessageBox::critical(this, tr("Error"), tr("The loaded identity does not have a type 2 block!"));
        return;
    }

    QString rescueCode;
    bool ok = showRescueCodeInputDialog(rescueCode);
    if (!ok) return;

    QByteArray decryptedIuk(32, 0);

    QProgressDialog progressDialog(tr("Decrypting identity unlock key..."), tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    ok = CryptUtil::decryptBlock2(
                decryptedIuk,
                pBlock2,
                rescueCode,
                &progressDialog);

    progressDialog.close();

    if (!ok)
    {
        QMessageBox::critical(this, tr("Error"), tr("Decryption of identity unlock key failed! Wrong password?"));
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

void MainWindow::decryptPreviousIuks()
{
    const QString DECRYPTION_METHOD_PASSWORD = tr("Password");
    const QString DECRYPTION_METHOD_RESCUE_CODE = tr("Rescue Code");
    QByteArray decryptedImk(32, 0);
    QByteArray decryptedIlk(32, 0);

    if (m_pIdentityModel == nullptr ||
        m_pIdentityModel->blocks.size() < 1)
    {
        showNoIdentityLoadedError();
        return;
    }

    IdentityBlock* pBlock3 = m_pIdentityModel->getBlock(3);
    if (pBlock3 == nullptr)
    {
        QMessageBox::critical(this, tr("Error"), tr("The loaded identity does not have a type 3 block!"));
        return;
    }

    bool ok = false;

    QString decryptionMethod = QInputDialog::getItem(
                this,
                tr(""),
                tr("Choose decrytion method:"),
                QStringList( { QString(DECRYPTION_METHOD_PASSWORD), QString(DECRYPTION_METHOD_RESCUE_CODE) } ),
                0,
                false,
                &ok);

    if (!ok) return;

    if (decryptionMethod == DECRYPTION_METHOD_PASSWORD)
    {
        IdentityBlock* pBlock1 = m_pIdentityModel->getBlock(1);
        if (pBlock1 == nullptr)
        {
            QMessageBox::critical(this, tr("Error"), tr("The loaded identity does not have a type 1 block!"));
            return;
        }

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

        QByteArray key = CryptUtil::createKeyFromPassword(
                    pBlock1,
                    password,
                    &progressDialog);

        progressDialog.close();

        if (!CryptUtil::decryptBlock1(
                    decryptedImk,
                    decryptedIlk,
                    pBlock1,
                    key))
        {
            QMessageBox::critical(this, tr("Error"), tr("Decryption of identity keys failed! Wrong password?"));
            return;
        }
    }
    else if (decryptionMethod == DECRYPTION_METHOD_RESCUE_CODE)
    {
        IdentityBlock* pBlock2 = m_pIdentityModel->getBlock(2);
        if (pBlock2 == nullptr)
        {
            QMessageBox::critical(this, tr("Error"), tr("The loaded identity does not have a type 2 block!"));
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

        QProgressDialog progressDialog(tr("Decrypting identity unlock key..."), tr("Abort"), 0, 0, this);
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
            QMessageBox::critical(this, tr("Error"), tr("Decryption of identity"
                                                        "unlock key failed! Wrong password?"));
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

void MainWindow::quit()
{
    QApplication::quit();
}

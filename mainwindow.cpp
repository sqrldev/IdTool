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

    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSaveIdentityFileAs, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ui->actionPasteIdentityData, &QAction::triggered, this, &MainWindow::pasteIdentityText);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::quit);
    connect(ui->actionCreateNewIdentity, &QAction::triggered, this, &MainWindow::createNewIdentity);
    connect(ui->actionBlockDesigner, &QAction::triggered, this, &MainWindow::showBlockDesigner);
    connect(ui->actionDecryptIdentityKeys, &QAction::triggered, this, &MainWindow::decryptIdentityKeys);
    connect(ui->actionCreateSiteKey, &QAction::triggered, this, &MainWindow::createSiteKey);
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
    QMessageBox msgBox(this);
    msgBox.critical(this, tr("Error"), tr("An identity needs to be loaded in order to complete this operation!"));
}

void MainWindow::openFile()
{
    QString dir = nullptr;

    const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
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
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Error"));
        msgBox.setText(e.what());
        msgBox.exec();
    }
}

void MainWindow::saveFile()
{
    QString dir = nullptr;

    const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
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
        QMessageBox msgBox(this);
        msgBox.setText(e.what());
        msgBox.exec();
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
    message.append("<a href=\"https://github.com/alexhauser/IdTool\">https://github.com/alexhauser/IdTool</a>");

    QMessageBox::about(this, "About", message);
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
            throw std::runtime_error(tr("Invalid identity data!")
                                     .toStdString());
        }

        m_pIdentityModel->clear();
        m_pIdentityParser->parseText(result, m_pIdentityModel);
        m_pUiBuilder->rebuild();
    }
    catch (std::exception& e)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Error"));
        msgBox.setText(e.what());
        msgBox.exec();
    }
}

void MainWindow::createNewIdentity()
{
    QString sBlockType;

    if (m_pUiBuilder->hasBlocks())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("This operation will discard any existing identity information!"));
        msgBox.setInformativeText(tr("Do you really want to create a new identity and discard all changes?"));
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);

        if (msgBox.exec() != QMessageBox::Ok) return;
    }

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

void MainWindow::createSiteKey()
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
                QLineEdit::Normal,
                "",
                &ok);

    if (!ok) return;

    QByteArray decryptedImk(32, 0);
    QByteArray decryptedIlk(32, 0);

    QProgressDialog progressDialog(tr("Decrypting identity keys..."), tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    if (!CryptUtil::decryptIdentityKeys(
                decryptedImk,
                decryptedIlk,
                pBlock,
                password,
                &progressDialog))
    {
        QMessageBox msgBox(this);
        msgBox.critical(this, tr("Error"), tr("Decryption of identity keys failed! Wrong password?"));
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
        QMessageBox msgBox(this);
        msgBox.critical(this, tr("Error"), tr("Creation of site keys failed, probably due to an error in the crypto routines!"));
        return;
    }

    QInputDialog resultDialog(this);
    resultDialog.setInputMode(QInputDialog::TextInput);
    resultDialog.setOption(QInputDialog::UsePlainTextEditForTextInput, true);
    resultDialog.resize(700, 250);
    resultDialog.setWindowTitle("Success");
    resultDialog.setLabelText("Creation of site keys succeeded!");
    resultDialog.setTextValue(QByteArray("Site-specific key:\n")
                              .append(publicKey.toHex()));
    resultDialog.exec();
}

void MainWindow::decryptIdentityKeys()
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

    QString password = QInputDialog::getText(
                nullptr,
                tr(""),
                tr("Identity password:"),
                QLineEdit::Normal,
                "",
                &ok);

    if (!ok) return;

    QByteArray decryptedImk(32, 0);
    QByteArray decryptedIlk(32, 0);

    QProgressDialog progressDialog(tr("Decrypting identity keys..."), tr("Abort"), 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);

    if (!CryptUtil::decryptIdentityKeys(
                decryptedImk,
                decryptedIlk,
                pBlock,
                password,
                &progressDialog))
    {
        QMessageBox msgBox(this);
        msgBox.critical(this, tr("Error"), tr("Decryption of identity keys failed! Wrong password?"));
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

void MainWindow::quit()
{
    QApplication::quit();
}

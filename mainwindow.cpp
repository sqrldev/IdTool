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
    connect(ui->actionCreateSiteKey, &QAction::triggered, this, &MainWindow::createSiteKey);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pIdentityParser;
    delete m_pIdentityModel;
    delete m_pUiBuilder;
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
        return;
    }

    IdentityBlock* pBlock = m_pIdentityModel->getBlock(1);
    if (pBlock == nullptr) return;

    IdentityBlockItem* pLengthItem = pBlock->getItem("Length");
    IdentityBlockItem* pTypeItem = pBlock->getItem("Type");
    IdentityBlockItem* pPlaintextLengthItem = pBlock->getItem("Plaintext length");
    IdentityBlockItem* pIvItem = pBlock->getItem("AES-GCM IV");
    IdentityBlockItem* pScryptSaltItem = pBlock->getItem("SCrypt random salt");
    IdentityBlockItem* pScryptLogNFactorItem = pBlock->getItem("SCrypt log(n) factor");
    IdentityBlockItem* pScryptIterationCountItem = pBlock->getItem("SCrypt iteration count");
    IdentityBlockItem* pOptionFlagsItem = pBlock->getItem("Option flags");
    IdentityBlockItem* pQuickPassLengthItem = pBlock->getItem("QuickPass length");
    IdentityBlockItem* pPassVerifySecItem = pBlock->getItem("Password verify seconds");
    IdentityBlockItem* pQuickPassTimeoutItem = pBlock->getItem("QuickPass timeout");
    IdentityBlockItem* pImkItem = pBlock->getItem("Identity Master Key");
    IdentityBlockItem* pVerificationTagItem = pBlock->getItem("Verification tag");

    QString pass = "test12345678";
    QByteArray imk = QByteArray::fromHex(pImkItem->value.toLocal8Bit());
    QByteArray iv = QByteArray::fromHex(pIvItem->value.toLocal8Bit());
    QByteArray scryptSalt = QByteArray::fromHex(pScryptSaltItem->value.toLocal8Bit());
    int logNFactor = pScryptLogNFactorItem->value.toInt();
    int iterationCount = pScryptIterationCountItem->value.toInt();
    QByteArray verificationTag = QByteArray::fromHex(pVerificationTagItem->value.toLocal8Bit());
    int plainTextLength = pPlaintextLengthItem->value.toInt();
    QByteArray plainText;
    plainText.append(pLengthItem->toByteArray());
    plainText.append(pTypeItem->toByteArray());
    plainText.append(pPlaintextLengthItem->toByteArray());
    plainText.append(pIvItem->toByteArray());
    plainText.append(pScryptSaltItem->toByteArray());
    plainText.append(pScryptLogNFactorItem->toByteArray());
    plainText.append(pScryptIterationCountItem->toByteArray());
    plainText.append(pOptionFlagsItem->toByteArray());
    plainText.append(pQuickPassLengthItem->toByteArray());
    plainText.append(pPassVerifySecItem->toByteArray());
    plainText.append(pQuickPassTimeoutItem->toByteArray());

    if (sodium_init() < 0)
    {
        QMessageBox msgBox(this);
        msgBox.setText(tr("Could not initialize sodium library. Aborting!"));
        msgBox.exec();
        return;
    }

    QByteArray derivedPwd = CryptUtil::enSCryptIterations(
                pass,
                scryptSalt,
                logNFactor,
                iterationCount);

    if (crypto_aead_aes256gcm_is_available() == 0) {
        return; /* Not available on this CPU */
    }

    unsigned char decrypted[1024];

    int ret = crypto_aead_aes256gcm_decrypt_detached(
                decrypted,
                nullptr,
                reinterpret_cast<const unsigned char*>(imk.constData()),
                static_cast<unsigned long long>(imk.length()),
                reinterpret_cast<const unsigned char*>(verificationTag.constData()),
                reinterpret_cast<const unsigned char*>(plainText.constData()),
                static_cast<unsigned long long>(plainTextLength),
                reinterpret_cast<const unsigned char*>(iv.constData()),
                reinterpret_cast<const unsigned char*>(derivedPwd.constData()));

    QString domain = "www.example.com";
    QByteArray domainBytes = domain.toLocal8Bit();
    unsigned char seed[crypto_sign_SEEDBYTES];

    ret = crypto_auth_hmacsha256(
                seed,
                reinterpret_cast<const unsigned char*>(domainBytes.constData()),
                static_cast<unsigned long long>(domainBytes.length()),
                reinterpret_cast<const unsigned char*>(imk.constData()));

    unsigned char publicKey[crypto_sign_PUBLICKEYBYTES];
    unsigned char secretKey[crypto_sign_SECRETKEYBYTES];

    ret = crypto_sign_seed_keypair(
                publicKey,
                secretKey,
                seed);
}

void MainWindow::quit()
{
    QApplication::quit();
}

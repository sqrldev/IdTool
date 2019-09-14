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
    m_pUiBuilder = new UiBuilder(m_pScrollArea, m_pIdentityModel);
    m_pHeaderFrame->setVisible(false);

    m_pScrollArea->setWidgetResizable(true);

    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSaveIdentityFileAs, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ui->actionPasteIdentityData, &QAction::triggered, this, &MainWindow::pasteIdentityText);
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
    QMessageBox messageBox(this);
    messageBox.setWindowTitle(tr("About"));
    messageBox.setText(tr("IdTool\n\nVersion: ") + APP_VERSION);
    messageBox.exec();
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

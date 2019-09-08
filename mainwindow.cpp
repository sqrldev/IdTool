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

    m_pHeaderFrame = this->findChild<QFrame*>("headerFrame");
    m_pScrollArea = this->findChild<QScrollArea*>("scrollArea");
    m_pIdentityParser = new IdentityParser();
    m_pIdentityModel = new IdentityModel();
    m_pUiBuilder = new UiBuilder(m_pScrollArea, m_pIdentityModel);
    m_pHeaderFrame->setVisible(false);

    m_pScrollArea->setWidgetResizable(true);

    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);
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
        tr("Open Image"), dir, tr("SQRL identity files (*.sqrl *.sqrc)"));

    if (fileName.isEmpty()) return;

    QByteArray ba = fileName.toLocal8Bit();
    char *pszFileName = ba.data();

    try
    {
        m_pIdentityParser->parseFile(pszFileName, m_pIdentityModel);
        m_pUiBuilder->build();

        //m_pHeaderFrame->setVisible(true);
    }
    catch (std::exception e)
    {
        QMessageBox msgBox;
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
        tr("Open Image"), dir, tr("SQRL identity files (*.sqrl *.sqrc)"));

    if (fileName.isEmpty()) return;

    try
    {
        m_pIdentityModel->writeToFile(fileName);
    }
    catch (std::exception e)
    {
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.exec();
    }
}

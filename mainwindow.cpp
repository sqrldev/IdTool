#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_pStorage != nullptr)
    {
        delete m_pStorage;
        m_pStorage = nullptr;
    }
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

    if (fileName == nullptr) return;

    QByteArray ba = fileName.toLocal8Bit();
    char *pszFileName = ba.data();

    if (m_pStorage != nullptr)
    {
        delete m_pStorage;
        m_pStorage = nullptr;
    }
    m_pStorage = new S4();
    m_pStorage->readIdentityFile(pszFileName);

}

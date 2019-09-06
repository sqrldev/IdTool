#include "mainwindow.h"
#include "s4parser.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QStandardPaths>

using json = nlohmann::json;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_pHeaderFrame = this->findChild<QFrame*>("headerFrame");
    m_pHeaderFrame->setVisible(false);

    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pIdentityModel;
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

    try
    {
        S4Parser parser;
        m_pIdentityModel = new IdentityModel();
        parser.parseIdentityFile(pszFileName, m_pIdentityModel);

        QScrollArea* scrollArea = this->findChild<QScrollArea*>("scrollArea");
        scrollArea->setWidgetResizable(true);
        UIBuilder builder(scrollArea, m_pIdentityModel);
        m_pHeaderFrame->setVisible(true);
        builder.build();
    }
    catch (std::exception e)
    {
        //TODO: Error handling
    }
}

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
    m_pScrollArea = this->findChild<QScrollArea*>("scrollArea");
    m_pParser = new S4Parser();
    m_pIdentityModel = new IdentityModel();
    m_pUiBuilder = new UIBuilder(m_pScrollArea, m_pIdentityModel);
    m_pHeaderFrame->setVisible(false);

    m_pScrollArea->setWidgetResizable(true);

    connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::openFile);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_pParser;
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

    if (fileName == nullptr) return;

    QByteArray ba = fileName.toLocal8Bit();
    char *pszFileName = ba.data();

    try
    {
        m_pParser->parseIdentityFile(pszFileName, m_pIdentityModel);
        m_pUiBuilder->build();

        //m_pHeaderFrame->setVisible(true);
    }
    catch (std::exception e)
    {
        //TODO: Error handling
    }
}

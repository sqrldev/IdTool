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


    S4Parser parser;
    IdentityModel model;
    parser.parseIdentityFile(pszFileName, &model);

    int i= 0;
    i++;

    /*

    if (m_pStorage != nullptr)
    {
        delete m_pStorage;
        m_pStorage = nullptr;
    }
    m_pStorage = new S4();
    m_pStorage->readIdentityFile(pszFileName);

    QScrollArea* scrollArea = this->findChild<QScrollArea*>("scrollArea");
    UIBuilder builder(this, scrollArea);
    builder.setStorage(m_pStorage);
    m_pHeaderFrame->setVisible(true);
    builder.build();




    std::ifstream t("C:\\Users\\alex.hauser\\Source\\Repos\\IdTool\\blockdef\\1.json");
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    json j = json::parse(str);
    std::string s = j["description"];

    // iterate the array
    for (size_t i=0; i<j["items"].size(); i++) {
      s = j["items"].at(i)["name"];
    }

    */
}

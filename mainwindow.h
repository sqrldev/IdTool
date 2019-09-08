#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "identitymodel.h"
#include "identityparser.h"
#include "uibuilder.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <QMainWindow>
#include <QScrollArea>
#include <QtGui>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openFile();
    void saveFile();

private:
    Ui::MainWindow *ui;
    QFrame* m_pHeaderFrame = nullptr;
    QScrollArea* m_pScrollArea = nullptr;
    IdentityModel *m_pIdentityModel = nullptr;
    IdentityParser* m_pIdentityParser = nullptr;
    UiBuilder* m_pUiBuilder = nullptr;

};

#endif // MAINWINDOW_H

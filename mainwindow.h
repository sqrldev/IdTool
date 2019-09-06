#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "identitymodel.h"
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

private:
    Ui::MainWindow *ui;
    QFrame* m_pHeaderFrame = nullptr;
    IdentityModel *m_pIdentityModel = nullptr;

};

#endif // MAINWINDOW_H

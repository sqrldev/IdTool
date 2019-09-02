#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "s4.hpp"
#include <QMainWindow>

using namespace SqrlStorage;

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
    S4 *m_pStorage = nullptr;

};

#endif // MAINWINDOW_H

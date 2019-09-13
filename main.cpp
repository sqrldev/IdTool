#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QRect screenGeometry = QGuiApplication::screens().first()->geometry(); //QApplication::desktop()->screenGeometry();
    int x = (screenGeometry.width() - w.width()) / 2;
    w.move(x, w.y() + 20);

    w.show();

    return a.exec();
}

#include <QtTest/QtTest>

class TestCryptUtil: public QObject
{
    Q_OBJECT
private:
    QList<QList<QByteArray>> parseVectorsCsv(QString fileName, bool skipFirstLine=true, bool removeQuotes=true);

private slots:
    void createSiteKeys();

};


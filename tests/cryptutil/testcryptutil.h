#include <QtTest/QtTest>

class TestCryptUtil: public QObject
{
    Q_OBJECT
private:
    QList<QList<QByteArray>> parseVectorsCsv(QString fileName, bool skipFirstLine=true, bool removeQuotes=true);

private slots:
    void reverseByteArray();
    void createSiteKeys();
    void enScryptIterations();
    void getHostLowercase();
    void makeHostLowercase();
    void enHash();
    void base56EncodeDecode();
    void base56EncodeDecodeRandomInput();
};


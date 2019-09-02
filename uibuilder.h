#ifndef UIBUILDER_H
#define UIBUILDER_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "s4.hpp"

using namespace SqrlStorage;

class UIBuilder
{
public:
    UIBuilder(QMainWindow* mainWindow, QWidget* container);
    void setStorage(S4* storage);
    void build();

private:
    QMainWindow* m_pMainWindow = nullptr;
    QWidget* m_pContainer = nullptr;
    S4* m_pStorage = nullptr;

    void buildBlockGeneric(S4::Block *block);
    void buildBlockType1(S4::BlockType1 *block);
    void buildBlockType2(S4::BlockType2 *block);
    void buildBlockType3(S4::BlockType3 *block);
    void addBlock(QString title, QColor color);
    void addLineItem(QString label, QString data, QVBoxLayout* layout);
};

#endif // UIBUILDER_H

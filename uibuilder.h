#ifndef UIBUILDER_H
#define UIBUILDER_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "identitymodel.h"

class UIBuilder
{
public:
    UIBuilder(QWidget* container, IdentityModel* model);
    void build();

private:
    QWidget* m_pContainer = nullptr;
    IdentityModel* m_pModel = nullptr;

    void buildBlock(IdentityModel::IdentityBlock* block);
    void addBlock(QString title, QColor color);
    void addLineItem(QString label, QString data, QVBoxLayout* layout);
};

#endif // UIBUILDER_H

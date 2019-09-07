#ifndef UIBUILDER_H
#define UIBUILDER_H

#include <QMainWindow>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include "identitymodel.h"

class UIBuilder
{
public:
    UIBuilder(QScrollArea* scrollArea, IdentityModel* model);
    void build();

private:
    QScrollArea* m_pScrollArea = nullptr;
    IdentityModel* m_pModel = nullptr;

    void buildBlock(IdentityModel::IdentityBlock* block);
    void addBlock(QString title, QColor color);
    QWidget* createBlockItem(QString label, QString data);
};

#endif // UIBUILDER_H

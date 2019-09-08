#ifndef UIBUILDER_H
#define UIBUILDER_H

#include <QMainWindow>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QUuid>
#include <QObjectUserData>
#include <QMessageBox>
#include <QInputDialog>
#include "identitymodel.h"

class UiBuilder : public QObject
{
    Q_OBJECT

public:
    UiBuilder(QScrollArea* scrollArea, IdentityModel* model);
    void build();

private:
    QScrollArea* m_pScrollArea = nullptr;
    IdentityModel* m_pModel = nullptr;

    QWidget* createBlock(IdentityModel::IdentityBlock* block);
    QWidget* createBlockHeader(IdentityModel::IdentityBlock *block);
    QWidget* createBlockItem(IdentityModel::IdentityBlockItem* item);

public slots:
    void editButtonClicked();

public:
    // Helper classes
    class EditButtonConnector : public QObjectUserData
    {
    public:
        IdentityModel::IdentityBlockItem* item = nullptr;

        EditButtonConnector(IdentityModel::IdentityBlockItem* item)
        {
            this->item = item;
        }
    };
};

#endif // UIBUILDER_H

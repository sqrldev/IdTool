#ifndef UIBUILDER_H
#define UIBUILDER_H

#include <QMainWindow>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QUuid>
#include <QObjectUserData>
#include <QMessageBox>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QToolTip>
#include <QPicture>
#include <QMenu>
#include "identitymodel.h"

class UiBuilder : public QObject
{    
    Q_OBJECT

public:
    UiBuilder(QScrollArea* scrollArea, IdentityModel* model);
    void rebuild();

private:
    QScrollArea* m_pScrollArea = nullptr;
    QWidget* m_pLastWidget = nullptr;
    QLayout* m_pLastLayout = nullptr;
    IdentityModel* m_pModel = nullptr;

    QWidget* createBlock(IdentityModel::IdentityBlock* block);
    QWidget* createBlockHeader(IdentityModel::IdentityBlock *block);
    QWidget* createBlockItem(IdentityModel::IdentityBlockItem* item);

public slots:
    void editButtonClicked();
    void copyButtonClicked();
    void blockOptionsButtonClicked();
    void deleteBlock();
    void moveBlock();

public:
    // Helper classes
    class BlockConnector
    {
    public:
        IdentityModel::IdentityBlock* block = nullptr;
        bool moveUp = true;

    public:
        BlockConnector();
        BlockConnector(BlockConnector& other);
        BlockConnector(IdentityModel::IdentityBlock* block, bool moveUp = true);
        ~BlockConnector();
    };

    class ItemConnector
    {
    public:
        IdentityModel::IdentityBlockItem* item = nullptr;
        QLineEdit* valueLabel = nullptr;

    public:
        ItemConnector();
        ItemConnector(ItemConnector& other);
        ItemConnector(IdentityModel::IdentityBlockItem* item, QLineEdit* valueLabel);
        ~ItemConnector();
    };
};

Q_DECLARE_METATYPE(UiBuilder::BlockConnector);
Q_DECLARE_METATYPE(UiBuilder::ItemConnector);

#endif // UIBUILDER_H

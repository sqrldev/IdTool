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
#include <QMetaType>
#include <QVariant>
#include <QDir>
#include "identitymodel.h"
#include "identityparser.h"

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

    QWidget* createBlock(IdentityBlock* block);
    QWidget* createBlockHeader(IdentityBlock *block);
    QWidget* createBlockItem(IdentityBlockItem* item);

public slots:
    void editButtonClicked();
    void copyButtonClicked();
    void blockOptionsButtonClicked();
    void addBlock();
    void deleteBlock();
    void moveBlock();
};

class BlockConnector
{
public:
    IdentityBlock* block = nullptr;
    bool moveUp = true;

public:
    BlockConnector();
    BlockConnector(const BlockConnector& other);
    BlockConnector(IdentityBlock* block, bool moveUp = true);
    ~BlockConnector();
};

class ItemConnector
{
public:
    IdentityBlockItem* item = nullptr;
    QLineEdit* valueLabel = nullptr;

public:
    ItemConnector();
    ItemConnector(const ItemConnector& other);
    ItemConnector(IdentityBlockItem* item, QLineEdit* valueLabel);
    ~ItemConnector();
};

Q_DECLARE_METATYPE(BlockConnector);
Q_DECLARE_METATYPE(ItemConnector);

#endif // UIBUILDER_H

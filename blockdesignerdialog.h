#ifndef BLOCKDESIGNERDIALOG_H
#define BLOCKDESIGNERDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSaveFile>
#include <QMessageBox>
#include <QJsonObject>
#include "identityparser.h"
#include "itemeditordialog.h"

namespace Ui {
class BlockDesignerDialog;
}

class BlockDesignerDialog : public QDialog
{
    Q_OBJECT

public:
    enum WorkMode
    {
        ADD,
        EDIT
    };

public:
    explicit BlockDesignerDialog(int blockType, QWidget *parent = nullptr);
    ~BlockDesignerDialog();

private:
    void createModelStub();
    void createBlockDefinition();
    bool loadBlockDefinition();
    void reload(bool reloadBlockDefinition);

private:
    Ui::BlockDesignerDialog *ui;
    WorkMode m_WorkMode;
    int m_BlockType = -1;
    QStandardItemModel* m_pItemModel = nullptr;
    QJsonDocument* m_pBlockDesign = nullptr;

public slots:
    void onAddItemClicked();
    void onDeleteItemClicked();
    void onMoveItemClicked();
    void onEditItemClicked();
    void onSaveButtonClicked();
    void onResetButtonClicked();
};

#endif // BLOCKDESIGNERDIALOG_H

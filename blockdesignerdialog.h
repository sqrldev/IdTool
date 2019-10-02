#ifndef BLOCKDESIGNERDIALOG_H
#define BLOCKDESIGNERDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "identityparser.h"

namespace Ui {
class BlockDesignerDialog;
}

class BlockDesignerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlockDesignerDialog(int blockType, QWidget *parent = nullptr);
    ~BlockDesignerDialog();
    void loadData();

private:
    Ui::BlockDesignerDialog *ui;
    int m_BlockType = -1;
    QStandardItemModel* m_pItemModel;
};

#endif // BLOCKDESIGNERDIALOG_H

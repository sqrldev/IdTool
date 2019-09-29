#ifndef BLOCKDESIGNERDIALOG_H
#define BLOCKDESIGNERDIALOG_H

#include <QDialog>

namespace Ui {
class BlockDesignerDialog;
}

class BlockDesignerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlockDesignerDialog(int blockType, QWidget *parent = nullptr);
    ~BlockDesignerDialog();

private:
    Ui::BlockDesignerDialog *ui;
    int m_BlockType = -1;
};

#endif // BLOCKDESIGNERDIALOG_H

#include "blockdesignerdialog.h"
#include "ui_blockdesignerdialog.h"

BlockDesignerDialog::BlockDesignerDialog(int blockType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockDesignerDialog)
{
    ui->setupUi(this);
    m_BlockType = blockType;
}

BlockDesignerDialog::~BlockDesignerDialog()
{
    delete ui;
}

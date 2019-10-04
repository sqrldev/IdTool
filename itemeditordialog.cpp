#include "itemeditordialog.h"
#include "ui_itemeditordialog.h"

ItemEditorDialog::ItemEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ItemEditorDialog)
{
    ui->setupUi(this);
}

ItemEditorDialog::~ItemEditorDialog()
{
    delete ui;
}

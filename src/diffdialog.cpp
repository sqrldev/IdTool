#include "diffdialog.h"
#include "ui_diffdialog.h"

DiffDialog::DiffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiffDialog)
{
    ui->setupUi(this);
}

DiffDialog::~DiffDialog()
{
    delete ui;
}

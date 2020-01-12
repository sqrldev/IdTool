#include "diffdialog.h"
#include "ui_diffdialog.h"

DiffDialog::DiffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiffDialog)
{
    this->setWindowFlags(Qt::Window);
    ui->setupUi(this);
}

DiffDialog::~DiffDialog()
{
    delete ui;
}

#ifndef ITEMEDITORDIALOG_H
#define ITEMEDITORDIALOG_H

#include <QDialog>

namespace Ui {
class ItemEditorDialog;
}

class ItemEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ItemEditorDialog(QWidget *parent = nullptr);
    ~ItemEditorDialog();

private:
    Ui::ItemEditorDialog *ui;
};

#endif // ITEMEDITORDIALOG_H

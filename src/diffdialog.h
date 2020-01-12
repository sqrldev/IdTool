#ifndef DIFFDIALOG_H
#define DIFFDIALOG_H

#include <QDialog>

namespace Ui {
class DiffDialog;
}

class DiffDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit DiffDialog(QWidget *parent = nullptr);
    ~DiffDialog();
    
private:
    Ui::DiffDialog *ui;
    
private slots:
    void onChooseIdentityFile();
};

#endif // DIFFDIALOG_H

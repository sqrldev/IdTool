#ifndef DIFFDIALOG_H
#define DIFFDIALOG_H

#include "common.h"

// Forward declarations
class IdentityModel;

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

private:
    QList<int> calculateColumnWidths(QList<IdentityModel*> ids);
    QTextCharFormat getItemFormat(int diffType=0);
    QTextCharFormat getHeaderFormat();
    
private slots:
    void onChooseIdentityFile();
    void onStartDiff();
};

#endif // DIFFDIALOG_H

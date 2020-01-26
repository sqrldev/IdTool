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
    QByteArray m_ImkId1;
    QByteArray m_IlkId1;
    QByteArray m_ImkId2;
    QByteArray m_IlkId2;
    QByteArray m_IukId1;
    QByteArray m_IukId2;

private:
    QList<int> calculateColumnWidths(QList<IdentityModel*> ids);
    QTextCharFormat getItemFormat(int diffType=0);
    QTextCharFormat getBlockHeaderFormat();
    QTextFrameFormat getBlockFrameFormat();
    QTextTableFormat getTableFormat();
    bool DecryptBlocks(QList<IdentityModel*>& ids);
    
private slots:
    void onChooseIdentityFile();
    void onStartDiff();
};

#endif // DIFFDIALOG_H

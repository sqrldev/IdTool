#ifndef ITEMEDITORDIALOG_H
#define ITEMEDITORDIALOG_H

#include <QDialog>
#include <QJsonObject>
#include "identitymodel.h"

namespace Ui {
class ItemEditorDialog;
}

class ItemEditorDialog : public QDialog
{
    Q_OBJECT

private:
    QJsonObject* m_pItem = nullptr;

public:
    explicit ItemEditorDialog(QWidget *parent = nullptr);
    ItemEditorDialog(QWidget *parent, QJsonObject* item);
    ~ItemEditorDialog();
    QJsonObject* getItem();

private:
    void loadDefaults();
    void loadItemData();

private:
    Ui::ItemEditorDialog *ui;
};

#endif // ITEMEDITORDIALOG_H

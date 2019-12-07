#ifndef TABMANAGER_H
#define TABMANAGER_H

#include "common.h"

// Forward declarations
class IdentityModel;
class UiBuilder;
class IdentityTab;

/**********************************************
 *    class TabManager                        *
 *********************************************/

class TabManager
{
private:
    QTabWidget* m_pTabWidget;
    QList<IdentityTab*> m_Tabs;

public:
    explicit TabManager(QTabWidget* tabWidget);
    int addTab(IdentityModel& identityModel, QFileInfo fileInfo, bool setActive = true);
    void removeTabAt(int index);
    IdentityTab& getTabAt(int index);
    IdentityTab &getActiveTab();
};

/**********************************************
 *    class IdentityTab                       *
 *********************************************/

class IdentityTab : public QWidget
{
    Q_OBJECT

private:
    QFileInfo m_FileInfo;
    QScrollArea* m_pScrollArea;
    IdentityModel* m_pIdentityModel;
    UiBuilder* m_pUiBuilder;

public:
    explicit IdentityTab(IdentityModel& identityModel, QFileInfo fileInfo, QWidget *parent = nullptr);
    IdentityModel& getIdentityModel();
    UiBuilder& getUiBuilder();
    void rebuild();
};

#endif // TABMANAGER_H

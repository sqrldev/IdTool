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

class TabManager : public QObject
{
    Q_OBJECT

private:
    QTabWidget* m_pTabWidget;
    QList<IdentityTab*> m_Tabs;
    bool m_bEnableUnauthenticatedChanges = false;

public:
    explicit TabManager(QTabWidget* tabWidget);
    int addTab(IdentityModel& identityModel, QFileInfo fileInfo, bool setActive = true);
    void removeTabAt(int index);
    IdentityTab& getTabAt(int index);
    IdentityTab &getCurrentTab();
    int getCurrentTabIndex();
    bool hasTabs();
    void setCurrentTabDirty(bool dirty);
    bool isCurrentTabDirty();
    void rebuildAllTabs();
    void setEnableUnauthenticatedChanges(bool enable, bool rebuild = true);
    void updateCurrentTabText();

signals:
    void currentTabChanged(int index);

private slots:
    void onTabCloseRequested(int index);
    void onCurrentTabChanged(int index);
};

/**********************************************
 *    class IdentityTab                       *
 *********************************************/

class IdentityTab : public QWidget
{
    Q_OBJECT

private:
    bool m_bIsDirty = false;
    QFileInfo m_FileInfo;
    QScrollArea* m_pScrollArea;
    IdentityModel* m_pIdentityModel;
    UiBuilder* m_pUiBuilder;

public:
    explicit IdentityTab(IdentityModel& identityModel, QFileInfo fileInfo, QWidget *parent = nullptr);
    void setDirty(bool dirty = true);
    bool isDirty();
    IdentityModel& getIdentityModel();
    UiBuilder& getUiBuilder();
    void rebuild();
    QString getTabText();
    QString getTabToolTip();
    void updateFileInfo(QFileInfo fileInfo);
};

#endif // TABMANAGER_H

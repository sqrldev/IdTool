#include "tabmanager.h"
#include "uibuilder.h"

TabManager::TabManager(QTabWidget *tabWidget)
{
    m_pTabWidget = tabWidget;

    connect(m_pTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
    connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));
}

int TabManager::addTab(IdentityModel &identityModel, QFileInfo fileInfo, bool setActive)
{
    IdentityTab* pTab= new IdentityTab(identityModel, fileInfo);
    pTab->getUiBuilder().setEnableUnauthenticatedChanges(m_bEnableUnauthenticatedChanges);

    connect(&(pTab->getUiBuilder()), SIGNAL(identityChanged()), this, SLOT(onCurrentIdentityChanged()));

    m_Tabs.append(pTab);
    m_pTabWidget->addTab(pTab, pTab->getTabText());

    int index = m_Tabs.size()-1;
    m_pTabWidget->setTabToolTip(index, fileInfo.filePath());
    if (setActive) m_pTabWidget->setCurrentIndex(index);

    return index;
}

IdentityTab &TabManager::getTabAt(int index)
{
    return *m_Tabs[index];
}

IdentityTab &TabManager::getCurrentTab()
{
    int currentIndex = m_pTabWidget->currentIndex();
    return *m_Tabs[currentIndex];
}

int TabManager::getCurrentTabIndex()
{
    return m_pTabWidget->currentIndex();
}

bool TabManager::hasTabs()
{
    return m_Tabs.count() > 0;
}

bool TabManager::hasDirtyTabs()
{
    for (IdentityTab* pTab : m_Tabs)
        if (pTab->isDirty()) return true;

    return false;
}

void TabManager::setCurrentTabDirty(bool dirty)
{
    if (!hasTabs()) return;
    getCurrentTab().setDirty(dirty);
    updateCurrentTabText();
}

bool TabManager::isCurrentTabDirty()
{
    if (!hasTabs()) return false;
    return getCurrentTab().isDirty();
}

void TabManager::rebuildAllTabs()
{
    for (IdentityTab* pTab : m_Tabs)
        pTab->rebuild();
}

void TabManager::setEnableUnauthenticatedChanges(bool enable, bool rebuild)
{
    m_bEnableUnauthenticatedChanges = enable;

    for (IdentityTab* pTab : m_Tabs)
        pTab->getUiBuilder().setEnableUnauthenticatedChanges(enable, rebuild);
}

void TabManager::updateCurrentTabText()
{
    if (!hasTabs()) return;

    m_pTabWidget->setTabText(getCurrentTabIndex(),
                             getCurrentTab().getTabText());

    m_pTabWidget->setTabToolTip(getCurrentTabIndex(),
                             getCurrentTab().getTabToolTip());
}

void TabManager::onTabCloseRequested(int index)
{
    if (m_Tabs[index]->isDirty())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(m_pTabWidget, tr("Unsaved changes"),
            tr("You have unsaved changes! Do you really want to close this identity "
               "without saving the changes?"));

        if (reply == QMessageBox::No)  return;
    }

    delete m_Tabs[index];
    m_Tabs.removeAt(index);

    emit currentTabChanged(getCurrentTabIndex());
}

void TabManager::onCurrentTabChanged(int index)
{
    emit currentTabChanged(index);
}

void TabManager::onCurrentIdentityChanged()
{
    if (!hasTabs()) return;
    setCurrentTabDirty(true);
}

IdentityTab::IdentityTab(IdentityModel& identityModel,
    QFileInfo fileInfo, QWidget *parent) :
    QWidget(parent)
{
    m_FileInfo = fileInfo;
    m_pIdentityModel = &identityModel;
    m_pScrollArea = new QScrollArea();
    m_pUiBuilder = new UiBuilder(m_pScrollArea, m_pIdentityModel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_pScrollArea);
    rebuild();
    setLayout(mainLayout);
}

void IdentityTab::setDirty(bool dirty)
{
    m_bIsDirty = dirty;
}

bool IdentityTab::isDirty()
{
    return m_bIsDirty;
}

IdentityModel &IdentityTab::getIdentityModel()
{
    return *m_pIdentityModel;
}

UiBuilder &IdentityTab::getUiBuilder()
{
    return *m_pUiBuilder;
}

void IdentityTab::rebuild()
{
    m_pUiBuilder->rebuild();
}

QString IdentityTab::getTabText()
{
    QString fileName = m_FileInfo.fileName();
    if (fileName == "") fileName = tr("Untitled");
    if (this->isDirty()) fileName += "*";

    return fileName;
}

QString IdentityTab::getTabToolTip()
{
    return m_FileInfo.filePath();
}

void IdentityTab::updateFileInfo(QFileInfo fileInfo)
{
    m_FileInfo = fileInfo;
}


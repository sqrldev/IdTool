#include "tabmanager.h"
#include "uibuilder.h"

TabManager::TabManager(QTabWidget *tabWidget)
{
    m_pTabWidget = tabWidget;
}

int TabManager::addTab(IdentityModel &identityModel, QFileInfo fileInfo, bool setActive)
{
    IdentityTab* pTab= new IdentityTab(identityModel, fileInfo);
    m_Tabs.append(pTab);
    m_pTabWidget->addTab(pTab, fileInfo.fileName());

    int index = m_Tabs.size()-1;
    m_pTabWidget->setTabToolTip(index, fileInfo.filePath());
    if (setActive) m_pTabWidget->setCurrentIndex(index);

    return index;
}

IdentityTab &TabManager::getTabAt(int index)
{
    return *m_Tabs[index];
}

IdentityTab &TabManager::getActiveTab()
{
    int currentIndex = m_pTabWidget->currentIndex();
    return *m_Tabs[currentIndex];
}

IdentityTab::IdentityTab(IdentityModel& identityModel, QFileInfo fileInfo, QWidget *parent) :
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



#include "tabmanager.h"
#include "uibuilder.h"
#include "identityclipboard.h"

/*!
 *
 * \class TabManager
 * \brief A class for managing multiple loaded identities in a tabbed
 * interface.
 *
 * \c TabManager holds a list of \c IdentityTab objects which are being
 * displayed using a \c QTabWidget. It provides methods for adding, removing
 * and managing tabs/identities.
 *
 * \sa IdentityTab
 *
*/


/*!
 * Creates a new \c TabManager object, storing a pointer to the \a tabWidget
 * that will host the identity tabs.
 */

TabManager::TabManager(QTabWidget *tabWidget)
{
    m_pTabWidget = tabWidget;
    m_pClipboard = IdentityClipboard::getInstance();

    connect(m_pTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));
    connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));
}

/*!
 * Adds a tab representing the visualization of \a identityModel to the \c QTabWidget.
 * Information about the identity file can be given via the \a fileInfo parameter, which
 * can be an empty \c QFileInfo object, if no file information is present (e.g. if the
 * identity was imported from the clipboard). If \a setActive is true, the newly added
 * tab will be activated, so that it is the currently selected tab.
 *
 * \returns Returns the index of the newly added tab.
 */

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

/*!
 * Returns the \c IdentityTab object stored at \a index in the list of tabs.
 */

IdentityTab &TabManager::getTabAt(int index)
{ 
    return *m_Tabs[index];
}

/*!
 * Returns the currently active \c IdentityTab object.
 */

IdentityTab &TabManager::getCurrentTab()
{
    int currentIndex = m_pTabWidget->currentIndex();
    return *m_Tabs[currentIndex];
}

/*!
 * Returns the index of the currently active \c IdentityTab object.
 */

int TabManager::getCurrentTabIndex()
{
    return m_pTabWidget->currentIndex();
}

/*!
 * Returns \c true if the \c TabManager instance currently hosts any
 * \c IdentityTab objects, or \c false otherwise.
 */

bool TabManager::hasTabs()
{
    return m_Tabs.count() > 0;
}

/*!
 * Returns \c true if the \c TabManager instance currently hosts any
 * dirty (changed but unsaved) \c IdentityTab objects, or \c false
 * otherwise.
 */

bool TabManager::hasDirtyTabs()
{
    for (IdentityTab* pTab : m_Tabs)
        if (pTab->isDirty()) return true;

    return false;
}

/*!
 * If \a dirty is \c true, marks the currently active \c IdentityTab
 * object as dirty (changed but unsaved), or clean otherwise.
 */

void TabManager::setCurrentTabDirty(bool dirty)
{
    if (!hasTabs()) return;
    getCurrentTab().setDirty(dirty);
    updateCurrentTabText();
}

/*!
 * Returns \c true if the currently active \c IdentityTab object is
 * dirty (changed but unsaved), or \c false otherwise.
 */

bool TabManager::isCurrentTabDirty()
{
    if (!hasTabs()) return false;
    return getCurrentTab().isDirty();
}

/*!
 * Rebuilds the visual representation of all tabs.
 */

void TabManager::rebuildAllTabs()
{
    for (IdentityTab* pTab : m_Tabs)
        pTab->rebuild();
}

/*!
 * \brief Sets an internal flag that signals whether unauthenticated changes
 * to the identity tabs should be enabled or disabled.
 *
 * Any settings and values within a SQRL identitiy are being protected against
 * unauthorized manipulation by applying the AES-GCM authenticated encryption.
 * With this method, the plain text within an identity is included with the
 * encryption of the identity keys as "additional data". Therefore, if any
 * information within an identity changes, the decryption of the keys will fail.
 *
 * "Unauthenticated changes" are changes to the identity which don't get
 * cryptographically "reauthenticated". This means, any well behaving client
 * will reject such "rogue" identities.
 *
 * If \a enable is \c true, unauthenticated changes are being enabled,
 * if it is set to \c false, they are being disabled.
 *
 * If \a rebuild is set to \c true, any change is immediately reflected
 * within the visual representation of the identity. For example, if
 * unauthenticated changes are being disabled and \a rebuild is \c true,
 * buttons for editing and moving identity blocks and items are removed
 * from the GUI.
 */

void TabManager::setEnableUnauthenticatedChanges(bool enable, bool rebuild)
{
    m_bEnableUnauthenticatedChanges = enable;

    for (IdentityTab* pTab : m_Tabs)
        pTab->getUiBuilder().setEnableUnauthenticatedChanges(enable, rebuild);
}

/*!
 * Updates the currently active tab's text and tooltip using information
 * provided by the tab itself.
 *
 * If file information is present for the current tab's identity, the file
 * name and path will be used for setting the tab's text and tooltip.
 * Otherwise, a name of "Untitled" will be used.
 *
 * An asteriks ("*") will be shown as a suffix to the tab text if the tab
 * is dirty (has unsaved changes).
 *
 * It is advisable to call this method after an operation that either
 * influences the "dirty state" of a tab, or the file information.
 *
 * \sa IdentityTab::getTabText(), IdentityTab::getTabToolTip()
 */

void TabManager::updateCurrentTabText()
{
    if (!hasTabs()) return;

    m_pTabWidget->setTabText(getCurrentTabIndex(),
                             getCurrentTab().getTabText());

    m_pTabWidget->setTabToolTip(getCurrentTabIndex(),
                             getCurrentTab().getTabToolTip());
}


/***************************************************
 *                S L O T S                        *
 * ************************************************/


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



/*!
 *
 * \class IdentityTab
 * \brief Represents a tab widget visualizing a single identity.
 *
 * \c IdentityTab uses a \c UiBuilder instance to render an \c IdentityModel
 * on a \c QScrollArea, which acts as the main "canvas" for drawing the visual
 * identity representation.
 *
 * It also holds a \c QFileInfo object, providing information about the source
 * of the identity, which is being used for displaying the tab's title text and
 * tooltip.
 *
 * \sa TabManager, UiBuilder, IdentityModel
 *
*/


/*!
 * Creates a new \c IdentityTab object, storing a references to the provided
 * \a identityModel and \a fileInfo.
 */

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

/*!
 * Marks the \c IdentityTab instance dirty, meaning that it has
 * unsaved changes.
 */

void IdentityTab::setDirty(bool dirty)
{
    m_bIsDirty = dirty;
}

/*!
 * Returns \c true if the \c IdentityTab instance is dirty, meaning
 * that it has unsaved changes, or \c false otherwise.
 */

bool IdentityTab::isDirty()
{
    return m_bIsDirty;
}

/*!
 * Returns the \c IdentityModel object that is being represented
 * by this \c IdentityTab instance.
 */

IdentityModel &IdentityTab::getIdentityModel()
{
    return *m_pIdentityModel;
}

/*!
 * Returns the \c UiBuilder object that is being used to render
 * this \c IdentityTab.
 */

UiBuilder &IdentityTab::getUiBuilder()
{
    return *m_pUiBuilder;
}

/*!
 * Rebuilds (redraws) the identity visualisation on this
 * \c IdentityTab.
 */

void IdentityTab::rebuild()
{
    m_pUiBuilder->rebuild();
}


/*!
 * Returns a string representing the identity file's name.
 * If no file information is present, a name of "Untitled"
 * will be returned.
 *
 * An asteriks ("*") will be added as a suffix to the tab
 * text if the tab is dirty (has unsaved changes).
 */

QString IdentityTab::getTabText()
{
    QString fileName = m_FileInfo.fileName();
    if (fileName == "") fileName = tr("Untitled");
    if (this->isDirty()) fileName += "*";

    return fileName;
}

/*!
 * Returns a string representing the identity file's full
 * file path. If no file information is present, an empty
 * string will be returned.
 */

QString IdentityTab::getTabToolTip()
{
    return m_FileInfo.filePath();
}

/*!
 * Updates the file info for the tab's identity with the
 * value provided in \a fileInfo.
 */

void IdentityTab::updateFileInfo(QFileInfo fileInfo)
{
    m_FileInfo = fileInfo;
}


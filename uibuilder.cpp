#include "uibuilder.h"

UiBuilder::UiBuilder(QScrollArea* scrollArea, IdentityModel* model)
{
    m_pScrollArea = scrollArea;
    m_pModel = model;
}

void UiBuilder::build()
{
    if (!m_pScrollArea || !m_pModel)
    {
        throw std::runtime_error("Invalid container or model pointers!");
    }

    QWidget* pWidget = new QWidget();
    QVBoxLayout *pLayout = new QVBoxLayout();
    pLayout->setSpacing(20);

    for (size_t i=0; i<m_pModel->blocks.size(); i++)
    {
        QWidget* pBlock = createBlock(&m_pModel->blocks[i]);
        pLayout->addWidget(pBlock);
    }

    pLayout->addStretch();

    pWidget->setLayout(pLayout);
    m_pScrollArea->setWidget(pWidget);
}

QWidget* UiBuilder::createBlock(IdentityModel::IdentityBlock *block)
{
    QString objectName = "obj_" + QUuid::createUuid().toString(QUuid::Id128);

    QFrame* pFrame = new QFrame();
    pFrame->setObjectName(objectName);
    pFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    QString styleSheet = QString("QFrame#") + objectName +
            " { background: " +
            block->color +
            "; border-radius: 6px; }";
    pFrame->setStyleSheet(styleSheet);
    QGridLayout* pFrameLayout = new QGridLayout();
    pFrameLayout->setSpacing(3);

    QWidget* pBlockHeader = createBlockHeader(block);
    pFrameLayout->addWidget(pBlockHeader);

    // Add items
    for (size_t i=0; i<block->items.size(); i++)
    {
        QWidget* pItemWidget = createBlockItem(&block->items[i]);
        pFrameLayout->addWidget(pItemWidget);
    }

    pFrame->setLayout(pFrameLayout);

    return pFrame;
}

QWidget* UiBuilder::createBlockHeader(IdentityModel::IdentityBlock *block)
{
    QWidget* pWidget = new QWidget();
    QHBoxLayout* pLayout = new QHBoxLayout();

    pLayout->setContentsMargins(10,10,10,30);

    QLabel* wLabel = new QLabel("Block " + block->description);
    QFont font = wLabel->font();
    font.setPointSize(14);
    wLabel->setFont(font);
    wLabel->setWordWrap(true);

    pLayout->addWidget(wLabel);
    pWidget->setLayout(pLayout);

    return pWidget;
}



QWidget* UiBuilder::createBlockItem(IdentityModel::IdentityBlockItem* item)
{
    QWidget* pWidget = new QWidget();
    QHBoxLayout* pLayout = new QHBoxLayout();
    QString value = item->value;

    if (value.length() > 50)
    {
        value = value.left(40);
        value += "...";
    }

    pLayout->setContentsMargins(0,0,0,0);

    QLabel* wLabel = new QLabel(item->name);
    wLabel->setWordWrap(true);
    wLabel->setMaximumWidth(150);
    wLabel->setMinimumWidth(150);
    pLayout->addWidget(wLabel);

    QLineEdit* wData = new QLineEdit(value);
    wData->setToolTip(item->value);
    wData->setToolTipDuration(-1);
    wData->setObjectName("wDataLabel");
    wData->setStyleSheet("QLineEdit#wDataLabel { background: rgb(237, 237, 237); border-radius: 6px; }");
    wData->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    wData->setTextMargins(5, 0, 5, 0);
    wData->setReadOnly(true);
    if (item->value.length() > 0) wData->setCursorPosition(0);
    pLayout->addWidget(wData);

    QPushButton* wButton = new QPushButton();
    wButton->setMaximumWidth(30);
    wLabel->setMinimumWidth(30);
    wButton->setIcon(QIcon(":/res/img/Edit_16x.png"));
    EditButtonConnector* pEditButtonConnector = new EditButtonConnector(item, wData);
    wButton->setUserData(0, pEditButtonConnector);
    connect(wButton, SIGNAL(clicked()), this, SLOT(editButtonClicked()));

    pLayout->addWidget(wButton);
    pWidget->setLayout(pLayout);

    return pWidget;
}

void UiBuilder::editButtonClicked()
{
    EditButtonConnector* pConnector =
            static_cast<EditButtonConnector*>(sender()->userData(0));

    bool ok = false;    
    QString result = QInputDialog::getMultiLineText(
                nullptr, tr("Edit value"), tr("New value for \"%1\":").arg(pConnector->item->name), pConnector->item->value, &ok);

    if (ok)
    {
        pConnector->item->value = result;
        pConnector->valueLabel->setText(result);
    }
}

UiBuilder::EditButtonConnector::EditButtonConnector(IdentityModel::IdentityBlockItem* item, QLineEdit* valueLabel)
{
    this->item = item;
    this->valueLabel = valueLabel;
}

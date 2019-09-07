#include "uibuilder.h"

UIBuilder::UIBuilder(QScrollArea* scrollArea, IdentityModel* model)
{
    m_pScrollArea = scrollArea;
    m_pModel = model;
}

void UIBuilder::build()
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

    pWidget->setLayout(pLayout);
    m_pScrollArea->setWidget(pWidget);
}

QWidget* UIBuilder::createBlock(IdentityModel::IdentityBlock *block)
{
    QString objectName = "obj_" + QUuid::createUuid().toString(QUuid::Id128);

    QFrame* pFrame = new QFrame();
    pFrame->setObjectName(objectName);
    pFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    QString styleSheet = "QFrame#";
    styleSheet += objectName;
    styleSheet += " { background: ";
    styleSheet += block->color.c_str();
    styleSheet += "; border-radius: 6px; }";
    pFrame->setStyleSheet(styleSheet);
    QGridLayout* pFrameLayout = new QGridLayout();
    pFrameLayout->setSpacing(3);

    QWidget* pBlockHeader = createBlockHeader(block);
    pFrameLayout->addWidget(pBlockHeader);

    // Add items
    for (size_t i=0; i<block->items.size(); i++)
    {
        QWidget* pItemWidget = createBlockItem(
                    QString::fromUtf8(block->items[i].name.c_str()),
                    QString::fromUtf8(block->items[i].value.c_str()));
        pFrameLayout->addWidget(pItemWidget);
    }

    pFrame->setLayout(pFrameLayout);

    return pFrame;
}

QWidget* UIBuilder::createBlockHeader(IdentityModel::IdentityBlock *block)
{
    QWidget* pWidget = new QWidget();
    QHBoxLayout* pLayout = new QHBoxLayout();

    pLayout->setContentsMargins(10,10,10,30);

    QLabel* wLabel = new QLabel("Block " + QString::fromUtf8(block->description.c_str()));
    QFont font = wLabel->font();
    font.setPointSize(14);
    wLabel->setFont(font);
    wLabel->setWordWrap(true);

    pLayout->addWidget(wLabel);
    pWidget->setLayout(pLayout);

    return pWidget;
}



QWidget* UIBuilder::createBlockItem(QString label, QString data)
{
    QWidget* pWidget = new QWidget();
    QHBoxLayout* pLayout = new QHBoxLayout();

    pLayout->setContentsMargins(0,0,0,0);

    QLabel* wLabel = new QLabel(label + ":");
    wLabel->setWordWrap(true);
    wLabel->setMaximumWidth(150);
    wLabel->setMinimumWidth(150);
    pLayout->addWidget(wLabel);

    QLabel* wData = new QLabel(data);
    wData->setObjectName("wDataLabel");
    wData->setStyleSheet("QLabel#wDataLabel { background: rgb(237, 237, 237); border-radius: 6px; }");
    wData->setMinimumWidth(300);
    wData->setMargin(5);
    wData->setWordWrap(true);
    pLayout->addWidget(wData);

    QPushButton* wButton = new QPushButton();
    wButton->setMaximumWidth(30);
    wLabel->setMinimumWidth(30);
    wButton->setIcon(QIcon(":/res/img/Edit_16x.png"));
    pLayout->addWidget(wButton);

    pWidget->setLayout(pLayout);

    return pWidget;
}

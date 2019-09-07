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
    //pLayout->setSizeConstraint(QLayout::SetFixedSize);

    // Add frame
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
    QFrame* pFrame = new QFrame();
    pFrame->setStyleSheet("background: rgb(214, 201, 163)");
    QGridLayout* pFrameLayout = new QGridLayout();
    pFrameLayout->setSpacing(3);

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
    wData->setStyleSheet("background: rgb(237, 237, 237)");
    wData->setMinimumWidth(300);
    wData->setMargin(5);
    wData->setWordWrap(true);
    pLayout->addWidget(wData);

    QPushButton* wButton = new QPushButton();
    wButton->setMaximumWidth(30);
    wLabel->setMinimumWidth(30);
    wButton->setIcon(QIcon(":/res/img/Paste_16x.png"));
    pLayout->addWidget(wButton);

    pWidget->setLayout(pLayout);

    return pWidget;
}

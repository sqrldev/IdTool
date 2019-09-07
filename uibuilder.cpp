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
        QFrame* pFrame = new QFrame();
        pFrame->setStyleSheet("background: rgb(214, 201, 163)");
        QGridLayout* pFrameLayout = new QGridLayout();

        // Add items
        for (int j=0; j<m_pModel->blocks[i].items.size(); j++)
        {
            QWidget* pItemWidget = createBlockItem(
                        QString::fromUtf8(m_pModel->blocks[i].items[j].name.c_str()),
                        QString::fromUtf8(m_pModel->blocks[i].items[j].value.c_str()));
            pFrameLayout->addWidget(pItemWidget);
        }


        pFrame->setLayout(pFrameLayout);
        pLayout->addWidget(pFrame);
    }



    pWidget->setLayout(pLayout);
    m_pScrollArea->setWidget(pWidget);

    /*
    QVBoxLayout *pLayout = new QVBoxLayout();
    pLayout->setSizeConstraint(QLayout::SetFixedSize);
    m_pScrollArea->setLayout(pLayout);

    for (size_t i=0; i<m_pModel->blocks.size(); i++)
    {
        buildBlock(&m_pModel->blocks[i]);
    }
    pLayout->insertStretch(0, 1);
    */
}



void UIBuilder::buildBlock(IdentityModel::IdentityBlock *block)
{
    /*
    QFrame* pBlockFrame = new QFrame();
    pBlockFrame->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    pBlockFrame->setStyleSheet("background: rgb(214, 201, 163)");

    QGridLayout* frameLayout = new QGridLayout();

    for (size_t i=0; i<block->items.size(); i++)
    {
        addLineItem(QString::fromUtf8(block->items[i].name.c_str()),
                    QString::fromUtf8(block->items[i].value.c_str()),
                    frameLayout);
    }

    pBlockFrame->setLayout(frameLayout);
    m_pScrollArea->layout()->addWidget(pBlockFrame);
    */
}



QWidget* UIBuilder::createBlockItem(QString label, QString data)
{
    QWidget* pWidget = new QWidget();
    //pWidget->setStyleSheet("background: rgb(66,66,66)");
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

#include "uibuilder.h"

UIBuilder::UIBuilder(QWidget* container, IdentityModel* model)
{
    m_pContainer = container;
    m_pModel = model;
}

void UIBuilder::build()
{
    if (!m_pContainer || !m_pModel)
    {
        throw std::runtime_error("Invalid container or model pointers!");
    }

    QVBoxLayout* pVerticalLayout = new QVBoxLayout();
    m_pContainer->setLayout(pVerticalLayout);

    for (size_t i=0; i<m_pModel->blocks.size(); i++)
    {
        buildBlock(&m_pModel->blocks[i]);
    }

    static_cast<QVBoxLayout*>(m_pContainer->layout())->addStretch();
}



void UIBuilder::buildBlock(IdentityModel::IdentityBlock *block)
{
    QFrame* pBlockFrame = new QFrame();
    pBlockFrame->setStyleSheet("background: rgb(214, 201, 163)");

    QVBoxLayout* pVerticalFrameLayout = new QVBoxLayout();

    addLineItem("Length:", QString::number(12), pVerticalFrameLayout);

    pVerticalFrameLayout->addStretch();

    pBlockFrame->setLayout(pVerticalFrameLayout);
    m_pContainer->layout()->addWidget(pBlockFrame);
}



void UIBuilder::addLineItem(QString label, QString data, QVBoxLayout* layout)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setAlignment(Qt::AlignTop);

    QLabel* wLabel = new QLabel(label);
    wLabel->setMaximumWidth(100);
    hlayout->addWidget(wLabel);

    QLabel* wData = new QLabel(data);
    hlayout->addWidget(wData);

    QPushButton* wButton = new QPushButton();
    wButton->setMaximumWidth(30);
    wButton->setIcon(QIcon(":/res/img/Paste_16x.png"));
    hlayout->addWidget(wButton);
    widget->setLayout(hlayout);

    layout->addWidget(widget);
}

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

    QGridLayout* pVerticalLayout = new QGridLayout();
    m_pContainer->setLayout(pVerticalLayout);

    for (size_t i=0; i<m_pModel->blocks.size(); i++)
    {
        buildBlock(&m_pModel->blocks[i]);
    }
}



void UIBuilder::buildBlock(IdentityModel::IdentityBlock *block)
{
    QFrame* pBlockFrame = new QFrame();
    pBlockFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    pBlockFrame->sizeHint();
    pBlockFrame->setStyleSheet("background: rgb(214, 201, 163)");

    QGridLayout* frameLayout = new QGridLayout();

    for (size_t i=0; i<block->items.size(); i++)
    {
        addLineItem(QString::fromUtf8(block->items[i].name.c_str()),
                    QString::fromUtf8(block->items[i].value.c_str()),
                    frameLayout);
    }

    pBlockFrame->setLayout(frameLayout);
    m_pContainer->layout()->addWidget(pBlockFrame);
}



void UIBuilder::addLineItem(QString label, QString data, QGridLayout* layout)
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

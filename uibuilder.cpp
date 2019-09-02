#include "uibuilder.h"

UIBuilder::UIBuilder(QMainWindow* mainWindow, QWidget* container)
{
    m_pMainWindow = mainWindow;
    m_pContainer = container;
}

void UIBuilder::setStorage(S4 *storage)
{
    m_pStorage = storage;
}

void UIBuilder::build()
{
    if (    m_pStorage == nullptr ||
            m_pMainWindow == nullptr ||
            m_pContainer == nullptr )
    {
        throw std::runtime_error("Invalid storage or main window pointers!");
    }

    QVBoxLayout* pVerticalLayout = new QVBoxLayout();
    pVerticalLayout->setMargin(0);
    m_pContainer->setLayout(pVerticalLayout);

    for (size_t i=0; i<m_pStorage->blocks.size(); i++)
    {
        switch (m_pStorage->blocks[i]->type) {

        case S4::EBlockType::TYPE_1_PW_ENCRYPTED_DATA:
            buildBlockType1(static_cast<S4::BlockType1*>(m_pStorage->blocks[i]));
            break;

        case S4::EBlockType::TYPE_2_RC_ENCRYPTED_DATA:
            buildBlockType2(static_cast<S4::BlockType2*>(m_pStorage->blocks[i]));
            break;

        case S4::EBlockType::TYPE_3_PREVIOUS_IUK:
            buildBlockType3(static_cast<S4::BlockType3*>(m_pStorage->blocks[i]));
            break;

        default:
            buildBlockGeneric(m_pStorage->blocks[i]);
            break;
        }
    }

    static_cast<QVBoxLayout*>(m_pContainer->layout())->addStretch();
}

void UIBuilder::buildBlockGeneric(S4::Block *block)
{

}

void UIBuilder::buildBlockType1(S4::BlockType1 *block)
{
    QFrame* pBlockFrame = new QFrame();
    pBlockFrame->setStyleSheet("background: rgb(214, 201, 163)");

    QVBoxLayout* pVerticalFrameLayout = new QVBoxLayout();

    addLineItem("Length:", QString::number(block->length), pVerticalFrameLayout);

    pVerticalFrameLayout->addStretch();

    pBlockFrame->setLayout(pVerticalFrameLayout);
    m_pContainer->layout()->addWidget(pBlockFrame);
}

void UIBuilder::buildBlockType2(S4::BlockType2 *block)
{

}

void UIBuilder::buildBlockType3(S4::BlockType3 *block)
{

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

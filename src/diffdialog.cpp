#include "diffdialog.h"
#include "ui_diffdialog.h"
#include "mainwindow.h"

DiffDialog::DiffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiffDialog)
{
    this->setWindowFlags(Qt::Window);
    ui->setupUi(this);

    QSize desktopSize = QDesktopWidget().availableGeometry(this).size() * 0.9;
    resize(geometry().width(), desktopSize.height());
    
    connect(ui->btn_Identity1, &QPushButton::clicked, this, &DiffDialog::onChooseIdentityFile);
    connect(ui->btn_Identity2, &QPushButton::clicked, this, &DiffDialog::onChooseIdentityFile);
    connect(ui->btn_StartDiff, &QPushButton::clicked, this, &DiffDialog::onStartDiff);
}

DiffDialog::~DiffDialog()
{
    delete ui;
}

QList<int> DiffDialog::calculateColumnWidths(QList<IdentityModel*> ids)
{
    QList<int> result {0,0};

    for (IdentityModel* id : ids)
    {
        for (IdentityBlock block : id->blocks)
        {
            for (IdentityBlockItem item : block.items)
            {
                if (item.name.length() > result[0]) result[0] = item.name.length();
                if (item.value.length() > result[1]) result[1] = item.value.length();
            }
        }
    }

    return result;
}

void DiffDialog::onChooseIdentityFile()
{
    QObject* pSender = sender();
    QString fileName = MainWindow::showChooseIdentityFileDialog(this);
    
    if (fileName == "") return;
    
    QLineEdit* txt = (pSender == ui->btn_Identity1) ? ui->txt_Identity1 : ui->txt_Identity2;
    txt->setText(fileName);
    
}

void DiffDialog::onStartDiff()
{
    QList<IdentityModel*> ids;
    ids.append(new IdentityModel());
    ids.append(new IdentityModel());

    IdentityParser parser;

    try
    {
        //TODO: Remove test code
        ui->txt_Identity1->setText("C:\\Users\\alexh\\Documents\\SQRL\\AlexDev#1.sqrl");
        ui->txt_Identity2->setText("C:\\Users\\alexh\\Documents\\SQRL\\AlexDev#1R2.sqrl");
        // End todo

        parser.parseFile(ui->txt_Identity1->text(), ids[0]);
        parser.parseFile(ui->txt_Identity2->text(), ids[1]);
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }

    QList<int> blockTypes1 = ids[0]->getAvailableBlockTypes();
    QList<int> blockTypes2 = ids[1]->getAvailableBlockTypes();
    QList<int> allBlockTypes = blockTypes1.toSet().unite(blockTypes2.toSet()).toList();
    std::sort(allBlockTypes.begin(), allBlockTypes.end());

    QList<int> columnWidths = calculateColumnWidths(ids);

    QTextDocument* textDoc = ui->txt_Diff->document();
    QTextFrame* rootFrame = textDoc->rootFrame();
    QTextCursor cursor = ui->txt_Diff->textCursor();

    QBrush backgroundBrush;
    backgroundBrush.setStyle(Qt::SolidPattern);
    backgroundBrush.setColor(QColor::fromRgb(255,0,0,150));

    QTextBlockFormat textBlockFormat;
    textBlockFormat.setBackground(backgroundBrush);
    textBlockFormat.setLeftMargin(5);

    for (int blockType : allBlockTypes)
    {
        cursor = textDoc->rootFrame()->lastCursorPosition();

        QTextFrameFormat frameFormat;
        frameFormat.setMargin(5);
        frameFormat.setPadding(5);
        cursor.insertFrame(frameFormat);

        cursor.insertHtml(
                QString("<h1>Block type %0</h1>").arg(blockType));

        IdentityBlock dummyBlock = IdentityParser::createEmptyBlock(blockType);
        int nrOfItems = dummyBlock.items.count();

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(3);
        tableFormat.setCellSpacing(1);
        tableFormat.setBorder(0);

        QTextCharFormat diffFormat;
        diffFormat.setBackground(backgroundBrush);

        QTextTable* pTable = cursor.insertTable(nrOfItems, 3, tableFormat);

        for (int i=0; i<nrOfItems; i++)
        {
            IdentityBlock* pBlockOfId1 = ids[0]->getBlock(blockType);
            IdentityBlock* pBlockOfId2 = ids[1]->getBlock(blockType);

            QString name = dummyBlock.items.at(i).name;
            QString value1 = pBlockOfId1 == nullptr ? "" : pBlockOfId1->items.at(i).value;
            QString value2 = pBlockOfId2 == nullptr ? "" : pBlockOfId2->items.at(i).value;

            QTextTableCell nameCell = pTable->cellAt(i, 0);
            cursor = nameCell.firstCursorPosition();
            cursor.insertText(name);

            QTextTableCell value1Cell = pTable->cellAt(i, 1);
            QTextCharFormat currentFormat = value1Cell.format();
            QTextCharFormat newFormat = currentFormat;
            newFormat.setBackground(Qt::lightGray);
            value1Cell.setFormat(newFormat);
            cursor = value1Cell.firstCursorPosition();
            cursor.insertText(value1);

            QTextTableCell value2Cell = pTable->cellAt(i, 2);
            cursor = value2Cell.firstCursorPosition();
            cursor.insertText(value2);
        }

        /*
        for (int i=0; i<nrOfItems; i++)
        {
            IdentityBlock* pBlockOfId1 = ids[0]->getBlock(blockType);
            IdentityBlock* pBlockOfId2 = ids[1]->getBlock(blockType);

            QString name = dummyBlock.items.at(i).name.leftJustified(columnWidths[0]);

            QString value1 = pBlockOfId1 == nullptr ? "" : pBlockOfId1->items.at(i).value;
            value1 = value1.rightJustified(columnWidths[1]);

            QString value2 = pBlockOfId2 == nullptr ? "" : pBlockOfId2->items.at(i).value;
            value2 = value2.leftJustified(columnWidths[1]);

            cursor.insertBlock();
            //cursor.setBlockFormat(textBlockFormat);
            cursor.insertHtml(QString("<pre>%0 : <pre>").arg(name));
            //cursor.setBlockFormat(textBlockFormat);
            cursor.insertHtml(QString("<pre>%1 | %2</pre>").arg(value1, value2));
        }
        */
    }
}

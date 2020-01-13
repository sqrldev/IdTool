#include "diffdialog.h"
#include "ui_diffdialog.h"
#include "mainwindow.h"

DiffDialog::DiffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiffDialog)
{
    this->setWindowFlags(Qt::Window);
    ui->setupUi(this);

    //QSize desktopSize = QDesktopWidget().availableGeometry(this).size() * 0.9;
    //resize(geometry().width(), desktopSize.height());
    this->setWindowState(Qt::WindowMaximized);
    
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

QTextCharFormat DiffDialog::getItemFormat(int diffType)
{
    QTextCharFormat result;
    QColor bgColor = QColor::fromRgb(255,255,255,0);

    if (diffType == 1) bgColor = QColor::fromRgb(156,219,156,255);
    if (diffType == 2) bgColor = QColor::fromRgb(255,135,135,255);
    if (diffType == 3) bgColor = QColor::fromRgb(255,255,224,255);

    result.setBackground(bgColor);
    result.setFont(QFont("Courier", 9));

    return result;
}

QTextCharFormat DiffDialog::getHeaderFormat()
{
    QTextCharFormat result;

    result.setFont(QFont("Courier", 16, QFont::Bold));
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
    QTextCursor cursor = ui->txt_Diff->textCursor();

    for (int blockType : allBlockTypes)
    {
        cursor = textDoc->rootFrame()->lastCursorPosition();

        QTextFrameFormat frameFormat;
        frameFormat.setMargin(5);
        frameFormat.setPadding(5);
        cursor.insertFrame(frameFormat);

        cursor.setCharFormat(getHeaderFormat());
        cursor.insertText(QString("Block type %0").arg(blockType));

        IdentityBlock dummyBlock = IdentityParser::createEmptyBlock(blockType);
        int nrOfItems = dummyBlock.items.count();

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(3);
        tableFormat.setCellSpacing(1);
        tableFormat.setBorder(0);

        QTextTable* pTable = cursor.insertTable(nrOfItems, 3, tableFormat);

        for (int i=0; i<nrOfItems; i++)
        {
            IdentityBlock* pBlockOfId1 = ids[0]->getBlock(blockType);
            IdentityBlock* pBlockOfId2 = ids[1]->getBlock(blockType);

            QString name = dummyBlock.items.at(i).name.leftJustified(columnWidths[0]);
            QString value1 = pBlockOfId1 == nullptr ? "" : pBlockOfId1->items.at(i).value;
            QString value2 = pBlockOfId2 == nullptr ? "" : pBlockOfId2->items.at(i).value;

            int diffType = 1;
            if (value1 != value2) diffType = 2;

            value1 = value1.rightJustified(columnWidths[1]);
            value2 = value2.leftJustified(columnWidths[1]);

            QTextTableCell nameCell = pTable->cellAt(i, 0);
            nameCell.setFormat(getItemFormat());
            cursor = nameCell.firstCursorPosition();
            cursor.insertText(name);

            QTextTableCell value1Cell = pTable->cellAt(i, 1);
            value1Cell.setFormat(getItemFormat(diffType));
            cursor = value1Cell.firstCursorPosition();
            cursor.insertText(value1);

            QTextTableCell value2Cell = pTable->cellAt(i, 2);
            value2Cell.setFormat(getItemFormat(diffType));
            cursor = value2Cell.firstCursorPosition();
            cursor.insertText(value2);
        }
    }
}

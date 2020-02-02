#include "diffdialog.h"
#include "ui_diffdialog.h"
#include "mainwindow.h"

DiffDialog::DiffDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiffDialog)
{
    this->setWindowFlags(Qt::Window);
    ui->setupUi(this);
    
    m_Ids = QList<IdentityModel*>();
    m_ImkId1 = QByteArray(32, 0);
    m_IlkId1 = QByteArray(32, 0);
    m_ImkId2 = QByteArray(32, 0);
    m_IlkId2 = QByteArray(32, 0);
    m_IukId1 = QByteArray(32, 0);
    m_IukId2 = QByteArray(32, 0);
    m_prevIuksId1 = QList<QByteArray>();
    m_prevIuksId2 = QList<QByteArray>();
    
    connect(ui->btn_Identity1, &QPushButton::clicked, this, &DiffDialog::onChooseIdentityFile);
    connect(ui->btn_Identity2, &QPushButton::clicked, this, &DiffDialog::onChooseIdentityFile);
    connect(ui->btn_StartDiff, &QPushButton::clicked, this, &DiffDialog::onStartDiff);
    connect(ui->chk_DecryptBlock1, SIGNAL(toggled(bool)), ui->txt_PassId1, SLOT(setEnabled(bool)));
    connect(ui->chk_DecryptBlock1, SIGNAL(toggled(bool)), ui->txt_PassId2, SLOT(setEnabled(bool)));
    connect(ui->chk_DecryptBlock2, SIGNAL(toggled(bool)), ui->txt_RescueCodeId1, SLOT(setEnabled(bool)));
    connect(ui->chk_DecryptBlock2, SIGNAL(toggled(bool)), ui->txt_RescueCodeId2, SLOT(setEnabled(bool)));
    
    //TODO: Remove test code
    ui->txt_Identity1->setText("C:\\Users\\Alex\\Documents\\SQRL\\AlexDev3_.sqrl");
    ui->txt_Identity2->setText("C:\\Users\\Alex\\Documents\\SQRL\\AlexDev3R4_.sqrl");
    ui->txt_PassId1->setText("test12345678");
    ui->txt_PassId2->setText("test12345678");
    ui->txt_RescueCodeId1->setText("4287-8546-8365-8491-4348-2554");
    ui->txt_RescueCodeId2->setText("5583-8638-9259-3876-2080-5033");
    // End todo
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

QTextCharFormat DiffDialog::getBlockHeaderFormat()
{
    QTextCharFormat result;

    result.setFont(QFont("Courier", 16, QFont::Bold));
    return result;
}

QTextCharFormat DiffDialog::getSummaryTextFormat()
{
    QTextCharFormat result;

    result.setFont(QFont("Courier", 12));
    return result;
}

QTextCharFormat DiffDialog::getSummarySuccessTextFormat()
{
    QTextCharFormat result = getSummaryTextFormat();
    result.setForeground(QBrush(QColor(156,219,156,255)));
    return result;
}

QTextCharFormat DiffDialog::getSummaryNeutralTextFormat()
{
    QTextCharFormat result = getSummaryTextFormat();
    result.setForeground(QBrush(QColor(255,255,224,255)));
    return result;
}

QTextCharFormat DiffDialog::getSummaryFailureTextFormat()
{
    QTextCharFormat result = getSummaryTextFormat();
    result.setForeground(QBrush(QColor(255,135,135,255)));
    return result;
}

QTextFrameFormat DiffDialog::getBlockFrameFormat()
{
    QTextFrameFormat frameFormat;
    frameFormat.setMargin(5);
    frameFormat.setPadding(5);
    return frameFormat;
}

QTextTableFormat DiffDialog::getTableFormat()
{
    QTextTableFormat tableFormat;
    tableFormat.setCellPadding(3);
    tableFormat.setCellSpacing(1);
    tableFormat.setBorder(0);
    return tableFormat;
}

bool DiffDialog::DecryptBlocks(QList<IdentityModel*>& ids)
{
    QString passwordId1, passwordId2, rescueCodeId1, rescueCodeId2;
    bool ok = false;
    
    if (ui->chk_DecryptBlock1->isChecked())
    {
        // Block 1
        
        passwordId1 = ui->txt_PassId1->text();
        passwordId2 = ui->txt_PassId2->text();
        
        IdentityBlock* pBlock1Id1 = ids[0]->getBlock(1);
        IdentityBlock* pBlock1Id2 = ids[1]->getBlock(1);
        
        QProgressDialog progressDialog(tr("Decrypting identity 1 block 1..."), tr("Abort"), 0, 0, this);
        progressDialog.setWindowModality(Qt::WindowModal);
        
        QByteArray keyId1(32, 0);
        QByteArray keyId2(32, 0);
        ok = CryptUtil::createKeyFromPassword(keyId1, *pBlock1Id1, passwordId1, &progressDialog);
        if (!ok) return false;
        progressDialog.setLabelText(tr("Decrypting identity 2 block 1..."));
        ok = CryptUtil::createKeyFromPassword(keyId2, *pBlock1Id2, passwordId2, &progressDialog);
        if (!ok) return false;
                       
        ok = CryptUtil::decryptBlock1(m_ImkId1, m_IlkId1, pBlock1Id1, keyId1);
        if (!ok) 
        {
            QMessageBox::critical(this, tr("Error"), tr("Decryption of block 1 of identity 1 failed!"));
            return false;
        }
        
        ok = CryptUtil::decryptBlock1(m_ImkId2, m_IlkId2, pBlock1Id2, keyId2);
        if (!ok) 
        {
            QMessageBox::critical(this, tr("Error"), tr("Decryption block 1 of identity 2 failed!"));
            return false;
        }
        
        // Block 3

        IdentityBlock* pBlock3Id1 = ids[0]->getBlock(3);
        IdentityBlock* pBlock3Id2 = ids[1]->getBlock(3);

        if (pBlock3Id1 != nullptr)
        {
            ok = CryptUtil::decryptBlock3(m_prevIuksId1, pBlock3Id1, m_ImkId1);
            if (!ok)
            {
                QMessageBox::critical(this, tr("Error"), tr("Decryption of block 3 of identity 1 failed!"));
                return false;
            }
        }

        if (pBlock3Id2 != nullptr)
        {
            ok = CryptUtil::decryptBlock3(m_prevIuksId2, pBlock3Id2, m_ImkId2);
            if (!ok)
            {
                QMessageBox::critical(this, tr("Error"), tr("Decryption of block 3 of identity 2 failed!"));
                return false;
            }
        }
    }
    
    // Block 2
    
    if (ui->chk_DecryptBlock2->isChecked())
    {
        rescueCodeId1 = ui->txt_RescueCodeId1->text();
        rescueCodeId2 = ui->txt_RescueCodeId2->text();

        // Block 2
        
        IdentityBlock* pBlock2Id1 = ids[0]->getBlock(2);
        IdentityBlock* pBlock2Id2 = ids[1]->getBlock(2);
        
        QProgressDialog progressDialog(tr("Decrypting identity 1 block 2..."), tr("Abort"), 0, 0, this);
        progressDialog.setWindowModality(Qt::WindowModal);
                       
        ok = CryptUtil::decryptBlock2(m_IukId1, pBlock2Id1, rescueCodeId1, &progressDialog);
        if (!ok) 
        {
            QMessageBox::critical(this, tr("Error"), tr("Decryption of block 2 of identity 1 failed!"));
            return false;
        }
        
        progressDialog.setLabelText(tr("Decrypting identity 2 block 2..."));
        ok = CryptUtil::decryptBlock2(m_IukId2, pBlock2Id2, rescueCodeId2, &progressDialog);
        if (!ok) 
        {
            QMessageBox::critical(this, tr("Error"), tr("Decryption of block 2 of identity 2 failed!"));
            return false;
        }
    }

    return true;
}

void DiffDialog::writeSummary(QTextCursor &cursor)
{
    QTextDocument* textDoc = ui->txt_Diff->document();

    cursor = textDoc->rootFrame()->lastCursorPosition();
    cursor.insertFrame(getBlockFrameFormat());

    cursor.setCharFormat(getBlockHeaderFormat());
    cursor.insertText(tr("Summary:"));
    
    cursor.insertBlock();
   
    if (m_IukId1 == m_IukId2) 
    {
        cursor.setCharFormat(getSummarySuccessTextFormat());
        cursor.insertText(tr("Both files represent the same identity!"));
    }
    else if (m_prevIuksId2.contains(m_IukId1) || m_prevIuksId1.contains(m_IukId2)) 
    {
        cursor.setCharFormat(getSummaryNeutralTextFormat());
        cursor.insertText(tr("Both files represent the same identity, but are not the same version!"));
    }
    else 
    {
        cursor.setCharFormat(getSummarySuccessTextFormat());
        cursor.insertText(tr("The files do NOT represent the same identity!"));
    }
    
    /*
  
    for (int i=0; i<2; i++)
    {
        IdentityModel* pThis = (i==0 ? m_Ids[0] : m_Ids[1]);
        IdentityModel* pOther = (i==0 ? m_Ids[1] : m_Ids[0]);
        
        QString nameOfThis = (i==0 ? tr("Identity 1") : tr("Identity 2"));
        QString nameOfOther = (i==0 ? tr("Identity 2") : tr("Identity 1"));
               
        cursor.insertBlock();
        cursor.setCharFormat(getSummaryTextFormat());
        
        // Print rekeying info, but do it only once
        if (i==0) 
        {
            int editionId1 = 0, editionId2 = 0;
            if (pThis->hasBlockType(3)) editionId1 = pThis->getBlock(3)->items[2].value.toInt();
            if (pOther->hasBlockType(3)) editionId2 = pOther->getBlock(3)->items[2].value.toInt();
            
            cursor.insertText(tr("%1 has been rekeyed %2 times.").arg(nameOfThis, QString::number(editionId1)));
            cursor.insertBlock();
            cursor.insertText(tr("%1 has been rekeyed %2 times.").arg(nameOfOther, QString::number(editionId2)));
            cursor.insertBlock();
        }      
    }
    */
    
}
    

void DiffDialog::writeDiffTable(QTextCursor &cursor, QList<int> &allBlockTypes)
{
    QTextDocument* textDoc = ui->txt_Diff->document();

    for (int blockType : allBlockTypes)
    {
        QList<int> columnWidths = calculateColumnWidths(m_Ids);

        cursor = textDoc->rootFrame()->lastCursorPosition();
        cursor.insertFrame(getBlockFrameFormat());

        cursor.setCharFormat(getBlockHeaderFormat());
        cursor.insertText(QString("Block type %0").arg(blockType));

        // Create a dummy block of the given block type as a
        // reference for the block type's items, since either
        // block 1 or block 2 could be missing a block that the
        // other one has.
        IdentityBlock dummyBlock = IdentityParser::createEmptyBlock(blockType);
        int nrOfItems = dummyBlock.items.count();

        QTextTable* pTable = cursor.insertTable(nrOfItems, 3, getTableFormat());

        for (int i=0; i<nrOfItems; i++)
        {
            IdentityBlock* pBlockOfId1 = m_Ids[0]->getBlock(blockType);
            IdentityBlock* pBlockOfId2 = m_Ids[1]->getBlock(blockType);

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
    IdentityParser parser;

    ui->txt_Diff->clear();
    m_prevIuksId1.clear();
    m_prevIuksId2.clear();

    m_Ids.clear();
    m_Ids.append(new IdentityModel());
    m_Ids.append(new IdentityModel());

    try
    {
        parser.parseFile(ui->txt_Identity1->text(), m_Ids[0]);
        parser.parseFile(ui->txt_Identity2->text(), m_Ids[1]);
    }
    catch (std::exception& e)
    {
        QMessageBox::critical(this, tr("Error"), e.what());
    }
    
    // Decrypt blocks if requested
    if (!DecryptBlocks(m_Ids)) return;

    // Get a sorted list of all the block types which are
    // present in both of the identities
    QList<int> blockTypesOfId1 = m_Ids[0]->getAvailableBlockTypes();
    QList<int> blockTypesOfId2 = m_Ids[1]->getAvailableBlockTypes();
    QList<int> allBlockTypes = blockTypesOfId1.toSet().unite(blockTypesOfId2.toSet()).toList();
    std::sort(allBlockTypes.begin(), allBlockTypes.end());

    QTextCursor cursor = ui->txt_Diff->textCursor();

    writeSummary(cursor);
    writeDiffTable(cursor, allBlockTypes);
    
    // Maximize diff window 
    this->setWindowState(Qt::WindowMaximized);
}

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
    m_prevImksId1 = QList<QByteArray>();
    m_prevImksId2 = QList<QByteArray>();
    
    connect(ui->btn_Identity1, &QPushButton::clicked, this, &DiffDialog::onChooseIdentityFile);
    connect(ui->btn_Identity2, &QPushButton::clicked, this, &DiffDialog::onChooseIdentityFile);
    connect(ui->btn_StartDiff, &QPushButton::clicked, this, &DiffDialog::onStartDiff);
    connect(ui->chk_DecryptBlock1, SIGNAL(toggled(bool)), ui->txt_PassId1, SLOT(setEnabled(bool)));
    connect(ui->chk_DecryptBlock1, SIGNAL(toggled(bool)), ui->txt_PassId2, SLOT(setEnabled(bool)));
    connect(ui->chk_DecryptBlock2, SIGNAL(toggled(bool)), ui->txt_RescueCodeId1, SLOT(setEnabled(bool)));
    connect(ui->chk_DecryptBlock2, SIGNAL(toggled(bool)), ui->txt_RescueCodeId2, SLOT(setEnabled(bool)));
    
    //TODO: Remove test code
    ui->txt_Identity1->setText("C:\\Users\\alexh\\Documents\\SQRL\\AlexDev3_.sqrl");
    ui->txt_Identity2->setText("C:\\Users\\alexh\\Documents\\SQRL\\AlexDev3R4_.sqrl");
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
                
                int itemlength = 0;
                
                if (ui->chk_ShortenKeys->isChecked()) itemlength = 23;
                else itemlength = item.value.length();
                if (itemlength > result[1]) result[1] = itemlength;
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

    result.setFont(QFont("Courier", 10));
    return result;
}

QTextCharFormat DiffDialog::getSummarySuccessTextFormat()
{
    QTextCharFormat result = getSummaryTextFormat();
    result.setBackground(QBrush(QColor(156,219,156,255)));
    return result;
}

QTextCharFormat DiffDialog::getSummaryNeutralTextFormat()
{
    QTextCharFormat result = getSummaryTextFormat();
    result.setBackground(QBrush(QColor(255,255,224,255)));
    return result;
}

QTextCharFormat DiffDialog::getSummaryFailureTextFormat()
{
    QTextCharFormat result = getSummaryTextFormat();
    result.setBackground(QBrush(QColor(255,135,135,255)));
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

QString DiffDialog::shortenValue(QString value)
{
    if (value.length() <= 23) return value;
    
    value = value.remove(10, value.length() - 20);
    value = value.insert(10, QString("..."));
    return value;
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
        
        // Insert the decrypted keys as block items so that they get displayed in the diff table
        IdentityBlockItem imkItem = IdentityBlockItem();
        imkItem.name = "  └ decrypted";
        imkItem.description = "The decrypted identity master key";
        imkItem.dataType = ItemDataType::BYTE_ARRAY;
        imkItem.nrOfBytes = 32;
        imkItem.value = m_ImkId1.toHex();
        m_Ids[0]->getBlock(1)->items.insert(12, imkItem);
        imkItem.value = m_ImkId2.toHex();
        m_Ids[1]->getBlock(1)->items.insert(12, imkItem);
        
        IdentityBlockItem ilkItem = IdentityBlockItem();
        ilkItem.name = "  └ decrypted";
        ilkItem.description = "The decrypted identity lock key";
        ilkItem.dataType = ItemDataType::BYTE_ARRAY;
        ilkItem.nrOfBytes = 32;
        ilkItem.value = m_IlkId1.toHex();
        m_Ids[0]->getBlock(1)->items.insert(14, ilkItem);
        ilkItem.value = m_IlkId2.toHex();
        m_Ids[1]->getBlock(1)->items.insert(14, ilkItem);
        
        // Block 3

        IdentityBlock* pBlock3Id1 = ids[0]->getBlock(3);
        IdentityBlock* pBlock3Id2 = ids[1]->getBlock(3);
        QList<QByteArray> prevIuksId1 = QList<QByteArray>();
        QList<QByteArray> prevIuksId2 = QList<QByteArray>();

        if (pBlock3Id1 != nullptr)
        {
            
            ok = CryptUtil::decryptBlock3(prevIuksId1, pBlock3Id1, m_ImkId1);
            if (!ok)
            {
                QMessageBox::critical(this, tr("Error"), tr("Decryption of block 3 of identity 1 failed!"));
                return false;
            }
            
            for (QByteArray iuk : prevIuksId1) m_prevImksId1.append(CryptUtil::createImkFromIuk(iuk));
        }

        if (pBlock3Id2 != nullptr)
        {
            ok = CryptUtil::decryptBlock3(prevIuksId2, pBlock3Id2, m_ImkId2);
            if (!ok)
            {
                QMessageBox::critical(this, tr("Error"), tr("Decryption of block 3 of identity 2 failed!"));
                return false;
            }
            
            for (QByteArray iuk : prevIuksId2) m_prevImksId2.append(CryptUtil::createImkFromIuk(iuk));
        }
        
        // Insert the decrypted previous keys as block items so that they get displayed in the diff table
        int maxPrevIuks = std::max(prevIuksId1.count(), prevIuksId2.count());
        for (int i=0; i<maxPrevIuks; i++)
        {
            IdentityBlockItem prevIukItem = IdentityBlockItem();
            prevIukItem.name = "  └ decrypted";
            prevIukItem.description = "The decrypted previous identity unlock key";
            prevIukItem.dataType = ItemDataType::BYTE_ARRAY;
            prevIukItem.nrOfBytes = 32;
            if (i < prevIuksId1.count())
            {
                prevIukItem.value = prevIuksId1[i].toHex();
                m_Ids[0]->getBlock(3)->items.insert(4+(i*2), prevIukItem);
            }
            if (i < prevIuksId2.count())
            {
                prevIukItem.value = prevIuksId2[i].toHex();
                m_Ids[1]->getBlock(3)->items.insert(4+(i*2), prevIukItem);    
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
        
        // Insert the decrypted IUK as block item so that it gets displayed in the diff table
        IdentityBlockItem iukItem = IdentityBlockItem();
        iukItem.name = "  └ decrypted";
        iukItem.description = "The decrypted identity unlock key";
        iukItem.dataType = ItemDataType::BYTE_ARRAY;
        iukItem.nrOfBytes = 32;
        iukItem.value = m_IukId1.toHex();
        m_Ids[0]->getBlock(2)->items.insert(6, iukItem);
        iukItem.value = m_IukId2.toHex();
        m_Ids[1]->getBlock(2)->items.insert(6, iukItem);
    }

    return true;
}

void DiffDialog::writeSummary(QTextCursor &cursor)
{
    if (!ui->chk_DecryptBlock1->isChecked()) return;

    QTextDocument* textDoc = ui->txt_Diff->document();
    cursor = textDoc->rootFrame()->lastCursorPosition();

    cursor.insertFrame(getBlockFrameFormat());
    cursor.setCharFormat(getBlockHeaderFormat());
    cursor.insertText(tr("Summary:"));
    
    cursor.insertBlock();

    if (m_ImkId1 == m_ImkId2)
    {
        cursor.setCharFormat(getSummarySuccessTextFormat());
        cursor.insertText(tr("Both files represent the same identity (decrypted IMKs match)!"));
    }
    else if (m_prevImksId2.contains(m_ImkId1) || m_prevImksId1.contains(m_ImkId2))
    {
        cursor.setCharFormat(getSummaryNeutralTextFormat());
        cursor.insertText(tr("Both files represent the same identity, but are not the same edition!\n"));
        if (m_prevImksId2.contains(m_ImkId1))
            cursor.insertText(tr("(IUK of Identity 1 is a previous IUK of Identity 2)\n"));
        if (m_prevImksId1.contains(m_ImkId2))
            cursor.insertText(tr("(IUK of Identity 2 is a previous IUK of Identity 1)\n"));
    }
    else
    {
        cursor.setCharFormat(getSummaryFailureTextFormat());
        cursor.insertText(tr("The files do NOT represent the same identity!"));
    }
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
        IdentityBlock* pDummyBlock = m_Ids[0]->getBlock(blockType) != nullptr ? 
                m_Ids[0]->getBlock(blockType) : m_Ids[1]->getBlock(blockType);
        int nrOfItems = pDummyBlock->items.count();

        QTextTable* pTable = cursor.insertTable(nrOfItems, 3, getTableFormat());

        for (int i=0; i<nrOfItems; i++)
        {
            IdentityBlock* pBlockOfId1 = m_Ids[0]->getBlock(blockType);
            IdentityBlock* pBlockOfId2 = m_Ids[1]->getBlock(blockType);

            QString name = pDummyBlock->items.at(i).name.leftJustified(columnWidths[0]);
            
            QString value1;
            QString value2;
            if (ui->chk_ShortenKeys->isChecked())
            {
                value1 = pBlockOfId1 == nullptr ? "" : shortenValue(pBlockOfId1->items.at(i).value);
                value2 = pBlockOfId2 == nullptr ? "" : shortenValue(pBlockOfId2->items.at(i).value);
            }
            else
            {
                value1 = pBlockOfId1 == nullptr ? "" : pBlockOfId1->items.at(i).value;
                value2 = pBlockOfId2 == nullptr ? "" : pBlockOfId2->items.at(i).value;
            }

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
    m_prevImksId1.clear();
    m_prevImksId2.clear();

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
    
    // Maximize diff window if "shorten keys" is disabled
    if (!ui->chk_ShortenKeys->isChecked())
        this->setWindowState(Qt::WindowMaximized);
}

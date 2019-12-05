/*
 * This file is part of the "IdTool" utility app.
 *
 * MIT License
 *
 * Copyright (c) 2019 Alexander Hauser
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"
#include "identitymodel.h"
#include "identityparser.h"
#include "uibuilder.h"
#include "blockdesignerdialog.h"
#include "idsetdialog.h"
#include "cryptutil.h"

namespace Ui {
class MainWindow;
}

/**********************************************
 *    class MainWindow                        *
 *********************************************/

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    QFrame* m_pHeaderFrame = nullptr;
    QScrollArea* m_pScrollArea = nullptr;
    IdentityModel *m_pIdentityModel = nullptr;
    IdentityParser* m_pIdentityParser = nullptr;
    UiBuilder* m_pUiBuilder = nullptr;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void showNoIdentityLoadedError();
    void showTextualIdentityInfoDialog(QString rescueCode = nullptr);
    bool showRescueCodeInputDialog(QString& rescueCode);
    bool showGetNewPasswordDialog(QString& password);
    bool canDiscardCurrentIdentity();

private slots:
    void onCreateNewIdentity();
    void onDisplayTextualIdentity();
    void onImportTextualIdentity();
    void onOpenFile();
    void onSaveFile();
    void onShowAboutDialog();
    void onChangePassword();
    void onShowIdentitySettingsDialog();
    void onControlUnauthenticatedChanges();
    void onPasteIdentityText();
    void onBuildNewIdentity();
    void onShowBlockDesigner();
    void onCreateSiteKeys();
    void onDecryptImkIlk();
    void onDecryptIuk();
    void onDecryptPreviousIuks();
    void onQuit();
};

#endif // MAINWINDOW_H

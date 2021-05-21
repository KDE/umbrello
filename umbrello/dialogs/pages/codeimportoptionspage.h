/*
    SPDX-FileCopyrightText: 2011-2014 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CODEIMPORTOPTIONSPAGE_H
#define CODEIMPORTOPTIONSPAGE_H

#include "dialogpagebase.h"
#include "ui_codeimportoptionspage.h"

/**
 * This class is used in the code generation wizard and
 * also in SettingsDialog.
 *
 * @author Ralf Habacker
 */
class CodeImportOptionsPage : public DialogPageBase, private Ui::CodeImportOptionsPage
{
    Q_OBJECT
public:
    explicit CodeImportOptionsPage(QWidget *parent = 0);
    ~CodeImportOptionsPage();
    void setDefaults();
    void apply();

signals:
     void applyClicked();

protected:
};

#endif

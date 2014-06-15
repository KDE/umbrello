/***************************************************************************
 * Copyright (C) 2011-2014 by Ralf Habacker <ralf.habacker@freenet.de>     *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

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

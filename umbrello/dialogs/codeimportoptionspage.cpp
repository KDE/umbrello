/***************************************************************************
 * Copyright (C) 2011 by Ralf Habacker <ralf.habacker@gmail.com>           *
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

// own header
#include "codeimportoptionspage.h"

// app includes
#include "optionstate.h"

// qt includes


/**
 * Constructor.
 * @param parent   the parent (wizard) of this wizard page
 */
CodeImportOptionsPage::CodeImportOptionsPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    createArtifactCheckBox->setChecked(Settings::optionState().codeImportState.createArtifacts);
}

/**
 * destructor
 */
CodeImportOptionsPage::~CodeImportOptionsPage()
{
}

/**
 * sets default values
 */
void CodeImportOptionsPage::setDefaults()
{
    createArtifactCheckBox->setChecked(true);
}

/**
 * Reads the set values from their corresponding widgets, writes them back to
 * the data structure, and notifies clients.
 */
void CodeImportOptionsPage::apply()
{
    Settings::optionState().codeImportState.createArtifacts = createArtifactCheckBox->isChecked();
    emit applyClicked();
}

/*
    SPDX-FileCopyrightText: 2011-2014 Ralf Habacker <ralf.habacker@freenet.de>

    This is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2, or (at your option)
    any later version.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this package; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

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
  : DialogPageBase(parent)
{
    setupUi(this);
    createArtifactCheckBox->setChecked(Settings::optionState().codeImportState.createArtifacts);
    resolveDependenciesCheckBox->setChecked(Settings::optionState().codeImportState.resolveDependencies);
    supportCPP11CheckBox->setChecked(Settings::optionState().codeImportState.supportCPP11);
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
    Settings::CodeImportState dummy;
    createArtifactCheckBox->setChecked(dummy.createArtifacts);
    resolveDependenciesCheckBox->setChecked(dummy.resolveDependencies);
    supportCPP11CheckBox->setChecked(dummy.supportCPP11);
}

/**
 * Reads the set values from their corresponding widgets, writes them back to
 * the data structure, and notifies clients.
 */
void CodeImportOptionsPage::apply()
{
    Settings::optionState().codeImportState.createArtifacts = createArtifactCheckBox->isChecked();
    Settings::optionState().codeImportState.resolveDependencies = resolveDependenciesCheckBox->isChecked();
    Settings::optionState().codeImportState.supportCPP11 = supportCPP11CheckBox->isChecked();
    emit applyClicked();
}

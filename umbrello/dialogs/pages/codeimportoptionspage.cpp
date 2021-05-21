/*
    SPDX-FileCopyrightText: 2011-2014 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-or-later
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

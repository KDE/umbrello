/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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

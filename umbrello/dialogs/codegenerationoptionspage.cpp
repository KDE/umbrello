/***************************************************************************
                          codegenerationoptionspage.cpp  -  description
                             -------------------
    begin                : Thu Jul 25 2002
    copyright            : (C) 2002 by Luis De la Parra
    email                : luis@delaparra.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "codegenerationoptionspage.h"

//kde include
#include <kfiledialog.h>
#include <klocale.h>
#include <kdebug.h>
//qt include
#include <qlineedit.h>
#include <qlistview.h>
#include <qbuttongroup.h>

//app includes




CodeGenerationOptionsPage::CodeGenerationOptionsPage(const SettingsDlg::CodeGenState &state,
						     QDict<GeneratorInfo> ldict, QString activeLanguage,
						     QWidget *parent,
						     const char *name )
	:CodeGenerationOptionsBase(parent,name) {
	setState(state);

	//populate language list
	bool foundActive = false;
	QDictIterator<GeneratorInfo> it(ldict);
	int indexCounter = 0;
	for (it.toFirst(); it.current(); ++it) {
		m_SelectLanguageBox->insertItem(it.current()->language, indexCounter);
		if(activeLanguage == it.current()->language) {
			m_SelectLanguageBox->setCurrentItem(indexCounter);
			foundActive = true;
		}
		indexCounter++;
	}

	//if we could not find the active language, we try to fall back to C++
	if (!foundActive) {
		indexCounter = 0;
		for(it.toFirst(); it.current(); ++it) {
			if (it.current()->language == "Cpp") {
				m_SelectLanguageBox->setCurrentItem(indexCounter);
				activeLanguage = it.current()->language;
				break;
			}
		}
	}
	//last try... if we dont have a activeLanguage and we have no Cpp installed we just
	//take the first language we find as "active"
	if( activeLanguage.isEmpty() ) {
		m_SelectLanguageBox->setCurrentItem(0);
	}
}

CodeGenerationOptionsPage::~CodeGenerationOptionsPage()
{
}

void CodeGenerationOptionsPage::setState(const SettingsDlg::CodeGenState &state) {
	m_forceDoc->setChecked(state.forceDoc);
	m_forceSections->setChecked(state.forceSections);
	m_outputDir->setText(state.outputDir);
	m_includeHeadings->setChecked(state.includeHeadings);
	m_headingsDir->setText(state.headingsDir);
	m_overwriteGroup->setButton(state.overwritePolicy); //0 = overwrite, 1 = ask, 2 = change name

}

void CodeGenerationOptionsPage::state(SettingsDlg::CodeGenState &state) {
	state.forceDoc = m_forceDoc->isChecked();
	state.forceSections = m_forceSections->isChecked();
	state.outputDir = m_outputDir->text();
	state.includeHeadings = m_includeHeadings->isChecked();
	state.headingsDir = m_headingsDir->text();
	state.overwritePolicy = (CodeGenerator::OverwritePolicy)m_overwriteGroup->id(m_overwriteGroup->selected());

}

void CodeGenerationOptionsPage::setDefaults() {
}

void CodeGenerationOptionsPage::browseClicked() {
	QString button = sender()->name();
	QString dir = KFileDialog::getExistingDirectory();
	if(dir.isEmpty())
		return;
	if(button=="m_browseOutput")
		m_outputDir->setText(dir);
	else if(button=="m_browseHeadings")
		m_headingsDir->setText(dir);

}

QString CodeGenerationOptionsPage::getCodeGenerationLanguage() {
	return m_SelectLanguageBox->currentText();
}

#include "codegenerationoptionspage.moc"

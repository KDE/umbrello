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
#include <kdebug.h>
#include "codegenerationoptionspage.h"
#include "codegenerationpolicypage.h"
#include "defaultcodegenpolicypage.h"
#include "../generatorinfo.h"
#include "../uml.h"

//kde include
#include <knuminput.h>
#include <kfiledialog.h>
#include <klocale.h>

//qt include
#include <qlineedit.h>
#include <qlistview.h>
#include <qbuttongroup.h>

CodeGenerationOptionsPage::CodeGenerationOptionsPage( CodeGenerator * gen, 
						     QString activeLanguage,
						     QWidget *parent, 
						     const char *name )
	:CodeGenerationOptionsBase(parent,name) 
{
	init (gen, activeLanguage);
}

CodeGenerationOptionsPage::~CodeGenerationOptionsPage() { }

void CodeGenerationOptionsPage::init( CodeGenerator * gen,
                                      QString activeLanguage
                                    )
{

	m_pCodeGenerator = 0;
	m_parentPolicy = 0;
	m_pCodePolicyPage = 0;

	if(gen)
		setCodeGenerator(gen);

	setupActiveLanguageBox(activeLanguage); 
}

void CodeGenerationOptionsPage::setCodeGenerator ( CodeGenerator * gen) {

	if(m_pCodeGenerator)
	{
		m_pCodeGenerator->disconnect();
		m_pCodeGenerator = 0;
	}

	m_parentPolicy = 0;

	CodeGenerationPolicy *policy = gen->getPolicy();
	m_parentPolicy = policy;
	m_pCodeGenerator = gen;


	m_forceDoc->setChecked(policy->getCodeVerboseDocumentComments());
	m_forceSections->setChecked(policy->getCodeVerboseSectionComments());

	m_outputDir->setText(policy->getOutputDirectory().absPath());
	m_includeHeadings->setChecked(policy->getIncludeHeadings());
	m_headingsDir->setText(policy->getHeadingFileDir());
	m_overwriteGroup->setButton(overwriteToInteger(policy->getOverwritePolicy()));

	m_SelectEndLineCharsBox->setCurrentItem(newLineToInteger(policy->getLineEndingType()));
	m_SelectIndentationTypeBox->setCurrentItem(indentTypeToInteger(policy->getIndentationType()));
	m_SelectIndentationNumber->setValue(policy->getIndentationAmount());

	connect(this,SIGNAL(syncCodeDocumentsToParent()),m_pCodeGenerator,SLOT(syncCodeToDocument()));

	// now insert the language-dependant page, should there be one
	updateCodeGenerationPolicyTab(gen);

}

void CodeGenerationOptionsPage::setupActiveLanguageBox(QString activeLanguage)
{
	GeneratorDict ldict = UMLApp::app()->generatorDict();
	//last but not least.. populate language list
	bool foundActive = false;
	GeneratorDictIt it(ldict);
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

int CodeGenerationOptionsPage::indentTypeToInteger(CodeGenerationPolicy::IndentationType value) {
       switch (value) {
                case CodeGenerationPolicy::SPACE:
                        return 2;
                case CodeGenerationPolicy::TAB:
                        return 1;
                default:
                case CodeGenerationPolicy::NONE:
                        return 0;
        }
}

int CodeGenerationOptionsPage::newLineToInteger(CodeGenerationPolicy::NewLineType value) {
        switch (value) {
                case CodeGenerationPolicy::DOS:
                        return 1;
                case CodeGenerationPolicy::MAC:
                        return 2;
                default:
                case CodeGenerationPolicy::UNIX:
                        return 0;
        }
}

//0 = overwrite, 1 = ask, 2 = change name
int CodeGenerationOptionsPage::overwriteToInteger(CodeGenerationPolicy::OverwritePolicy value) {
	switch (value) {
		case CodeGenerationPolicy::Ok:
			return 0;
		case CodeGenerationPolicy::Never:
			return 2;
		default:
		case CodeGenerationPolicy::Ask:
			return 1;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CodeGenerationOptionsPage::updateCodeGenerationPolicyTab(CodeGenerator * gen) {

	if(m_pCodePolicyPage) 
	{
		m_pCodePolicyPage->disconnect();
		m_pCodePolicyPage = 0;
	}

	if(!gen)
		m_pCodePolicyPage = new DefaultCodeGenPolicyPage(languageOptionsFrame, "codelangpolicypage");
	else
		m_pCodePolicyPage = gen->getPolicy()->createPage(languageOptionsFrame, "codelangpolicypage");

	connect(this,SIGNAL(applyClicked()),m_pCodePolicyPage,SLOT(apply()));

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CodeGenerationOptionsPage::apply() {

	if(m_parentPolicy) {

		m_parentPolicy->setCodeVerboseDocumentComments(m_forceDoc->isChecked());
		m_parentPolicy->setCodeVerboseSectionComments(m_forceSections->isChecked());
		m_parentPolicy->setOutputDirectory(QDir(m_outputDir->text()));
		m_parentPolicy->setIncludeHeadings(m_includeHeadings->isChecked());
		m_parentPolicy->setHeadingFileDir(m_headingsDir->text());
		m_parentPolicy->setOverwritePolicy((CodeGenerationPolicy::OverwritePolicy)m_overwriteGroup->id(m_overwriteGroup->selected())); 
		m_parentPolicy->setLineEndingType((CodeGenerationPolicy::NewLineType) m_SelectEndLineCharsBox->currentItem());
		m_parentPolicy->setIndentationType((CodeGenerationPolicy::IndentationType) m_SelectIndentationTypeBox->currentItem());
		m_parentPolicy->setIndentationAmount(m_SelectIndentationNumber->value());

		// emit in THIS order.. the first signal triggers any sub-class to do its apply
		// slot, THEN, once we are all updated, we may sync the parent generator's code
		// documents
		emit applyClicked();
		emit syncCodeDocumentsToParent();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CodeGenerationOptionsPage::activeLanguageChanged(int /*id*/)
{
	emit languageChanged();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CodeGenerationOptionsPage::setDefaults() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
QString CodeGenerationOptionsPage::getCodeGenerationLanguage() {
	return m_SelectLanguageBox->currentText();
}

#include "codegenerationoptionspage.moc"

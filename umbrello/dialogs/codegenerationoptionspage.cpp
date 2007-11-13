/***************************************************************************
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
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
// own header
#include "codegenerationoptionspage.h"
// qt/kde includes
#include <qcheckbox.h>
#include <kdebug.h>
// app includes
#include "../codegenerator.h"
#include "codegenerationpolicypage.h"
#include "../codegenerators/codegenfactory.h"
#include "../codegenerators/codegenpolicyext.h"
#include "defaultcodegenpolicypage.h"
#include "../model_utils.h"
#include "../uml.h"

//kde include
#include <knuminput.h>
#include <kfiledialog.h>
#include <klocale.h>

//qt include
#include <qlineedit.h>
#include <q3listview.h>
#include <q3buttongroup.h>

CodeGenerationOptionsPage::CodeGenerationOptionsPage(QWidget *parent)
    : QWidget( parent )
{
    setupUi(this);
    init();
}

CodeGenerationOptionsPage::~CodeGenerationOptionsPage() { }

void CodeGenerationOptionsPage::init()
{
    m_pCodePolicyPage = 0;

    CodeGenerationPolicy *policy = UMLApp::app()->getCommonPolicy();
    m_parentPolicy = policy;
    CodeGenerator *gen = UMLApp::app()->getGenerator();

    m_forceDoc->setChecked(policy->getCodeVerboseDocumentComments());
    m_forceSections->setChecked(policy->getCodeVerboseSectionComments());

    m_outputDir->setText(policy->getOutputDirectory().absolutePath());
    m_includeHeadings->setChecked(policy->getIncludeHeadings());
    m_headingsDir->setText(policy->getHeadingFileDir());
    m_overwriteGroup->setButton(overwriteToInteger(policy->getOverwritePolicy()));

    m_SelectEndLineCharsBox->setCurrentIndex(newLineToInteger(policy->getLineEndingType()));
    m_SelectIndentationTypeBox->setCurrentIndex(indentTypeToInteger(policy->getIndentationType()));
    m_SelectIndentationNumber->setValue(policy->getIndentationAmount());

    connect(this,SIGNAL(syncCodeDocumentsToParent()),gen,SLOT(syncCodeToDocument()));
    connect(this,SIGNAL(languageChanged()), this, SLOT(updateCodeGenerationPolicyTab()));

    setupActiveLanguageBox();

    // now insert the language-dependant page, should there be one
    updateCodeGenerationPolicyTab();
}

void CodeGenerationOptionsPage::setupActiveLanguageBox()
{
    int indexCounter = 0;
    while (indexCounter < Uml::pl_Reserved) {
        QString language = Model_Utils::progLangToString((Uml::Programming_Language) indexCounter);
        m_SelectLanguageBox->insertItem(indexCounter, language);
        indexCounter++;
    }
    m_SelectLanguageBox->setCurrentIndex(UMLApp::app()->getActiveLanguage());
    connect(m_SelectLanguageBox, SIGNAL(activated(int)), this, SLOT(activeLanguageChanged(int)));
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

void CodeGenerationOptionsPage::updateCodeGenerationPolicyTab() {

    if(m_pCodePolicyPage)
    {
        tabWidget->removeTab(2);
        m_pCodePolicyPage->disconnect();

        delete m_pCodePolicyPage;
        m_pCodePolicyPage = 0;
    }

    Uml::Programming_Language pl = (Uml::Programming_Language) m_SelectLanguageBox->currentIndex();
    CodeGenPolicyExt *policyExt = CodeGenFactory::newCodeGenPolicyExt(pl);

    //qDeleteAll( languageOptionsFrame->children() );

    if (policyExt)
        m_pCodePolicyPage = policyExt->createPage(0 , "codelangpolicypage");
    else
        m_pCodePolicyPage = new DefaultCodeGenPolicyPage(0 , "codelangpolicypage");

    tabWidget->insertTab(2, m_pCodePolicyPage, i18n("Language Options" ) );

    connect(this,SIGNAL(applyClicked()),m_pCodePolicyPage,SLOT(apply()));

}

void CodeGenerationOptionsPage::apply() {

    if(m_parentPolicy) {

        m_parentPolicy->setCodeVerboseDocumentComments(m_forceDoc->isChecked());
        m_parentPolicy->setCodeVerboseSectionComments(m_forceSections->isChecked());
        m_parentPolicy->setOutputDirectory(QDir(m_outputDir->text()));
        m_parentPolicy->setIncludeHeadings(m_includeHeadings->isChecked());
        m_parentPolicy->setHeadingFileDir(m_headingsDir->text());
        m_parentPolicy->setOverwritePolicy((CodeGenerationPolicy::OverwritePolicy)m_overwriteGroup->id(m_overwriteGroup->selected()));
        m_parentPolicy->setLineEndingType((CodeGenerationPolicy::NewLineType) m_SelectEndLineCharsBox->currentIndex());
        m_parentPolicy->setIndentationType((CodeGenerationPolicy::IndentationType) m_SelectIndentationTypeBox->currentIndex());
        m_parentPolicy->setIndentationAmount(m_SelectIndentationNumber->value());

        m_pCodePolicyPage->apply();
        // emit in THIS order.. the first signal triggers any sub-class to do its apply
        // slot, THEN, once we are all updated, we may sync the parent generator's code
        // documents
        emit applyClicked();
        emit syncCodeDocumentsToParent();
    }
}

void CodeGenerationOptionsPage::activeLanguageChanged(int /*id*/)
{
    emit languageChanged();
}

void CodeGenerationOptionsPage::setDefaults() {
}

void CodeGenerationOptionsPage::browseClicked() {

    QString button = sender()->objectName();
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

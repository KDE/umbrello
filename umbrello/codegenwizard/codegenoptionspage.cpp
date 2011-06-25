/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002                                                    *
 *   Luis De la Parra <luis@delaparra.org>                                 *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codegenoptionspage.h"

// app includes
#include "codegenerator.h"
#include "codegenerationpolicypage.h"
#include "codegenerators/codegenfactory.h"
#include "codegenerators/codegenpolicyext.h"
#include "defaultcodegenpolicypage.h"
#include "uml.h"

// kde includes
#include <knuminput.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>

// qt includes


/**
 * Constructor.
 * @param parent   the parent (wizard) of this wizard page
 */
CodeGenOptionsPage::CodeGenOptionsPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18n("Code Generation Options"));
    setSubTitle(i18n("Adjust code generation options."));

    setupUi(this);

    m_pCodePolicyPage = 0;
    m_parentPolicy = UMLApp::app()->commonPolicy();
    CodeGenerator* gen = UMLApp::app()->generator();

    ui_forceDoc->setChecked(m_parentPolicy->getCodeVerboseDocumentComments());
    ui_forceSections->setChecked(m_parentPolicy->getCodeVerboseSectionComments());

    ui_outputDir->setText(m_parentPolicy->getOutputDirectory().absolutePath());
    ui_includeHeadings->setChecked(m_parentPolicy->getIncludeHeadings());
    ui_headingsDir->setText(m_parentPolicy->getHeadingFileDir());
    overwriteToWidget(m_parentPolicy->getOverwritePolicy())->setChecked(true);

    ui_SelectEndLineCharsBox->setCurrentIndex(newLineToInteger(m_parentPolicy->getLineEndingType()));
    ui_SelectIndentationTypeBox->setCurrentIndex(indentTypeToInteger(m_parentPolicy->getIndentationType()));
    ui_SelectIndentationNumber->setValue(m_parentPolicy->getIndentationAmount());

    connect(this, SIGNAL(syncCodeDocumentsToParent()), gen, SLOT(syncCodeToDocument()));
    connect(this, SIGNAL(languageChanged()), this, SLOT(updateCodeGenerationPolicyTab()));
    connect(this, SIGNAL(languageChanged()), this, SLOT(changeLanguage()) );

    connect(ui_browseOutput, SIGNAL(clicked()), this, SLOT(browseClicked()));
    connect(ui_browseHeadings, SIGNAL(clicked()), this, SLOT(browseClicked()));

    setupActiveLanguageBox();

    //now insert the language-dependant page, should there be one
    updateCodeGenerationPolicyTab();
}

/**
 * Destructor.
 */
CodeGenOptionsPage::~CodeGenOptionsPage()
{
}

/**
 * Fills the language combo box with items and
 * sets the currently selected value.
 */
void CodeGenOptionsPage::setupActiveLanguageBox()
{
    int indexCounter = 0;
    while (indexCounter < Uml::ProgrammingLanguage::Reserved) {
        QString language = Uml::ProgrammingLanguage::toString(Uml::ProgrammingLanguage::Value(indexCounter));
        ui_SelectLanguageBox->insertItem(indexCounter, language);
        indexCounter++;
    }
    ui_SelectLanguageBox->setCurrentIndex(UMLApp::app()->activeLanguage());
    connect(ui_SelectLanguageBox, SIGNAL(activated(int)), this, SLOT(activeLanguageChanged(int)));
}

/**
 * Static utility function to convert the indentation type to integer.
 * @param value   the indentation type
 * @return        the corresponding integer value
 */
int CodeGenOptionsPage::indentTypeToInteger(CodeGenerationPolicy::IndentationType value)
{
    switch (value) {
    case CodeGenerationPolicy::NONE:
        return 0;
    case CodeGenerationPolicy::TAB:
        return 1;
    case CodeGenerationPolicy::SPACE:
        return 2;
    default:
        return 0;
    }
}

/**
 * Static utility function to convert the new line type to integer.
 * @param value   the new line type
 * @return        the corresponding integer value
 */
int CodeGenOptionsPage::newLineToInteger(CodeGenerationPolicy::NewLineType value) 
{
    switch (value) {
    case CodeGenerationPolicy::UNIX:
        return 0;
    case CodeGenerationPolicy::DOS:
        return 1;
    case CodeGenerationPolicy::MAC:
        return 2;
    default:
        return 0;
    }
}

/**
 * Converts the overwrite policy value to the corresponding widget object.
 * @param value   the overwrite policy
 * @return        the corresponding widget obeject
 */
QRadioButton* CodeGenOptionsPage::overwriteToWidget(CodeGenerationPolicy::OverwritePolicy value)
{
    switch (value) {
    case CodeGenerationPolicy::Ok:
        return ui_radioButtonOverwrite;
    case CodeGenerationPolicy::Ask:
        return ui_radioButtonAsk;
    case CodeGenerationPolicy::Never:
        return ui_radioButtonChangeName;
    default:
        return ui_radioButtonAsk;
    }
}

/**
 * Converts the corresponding widget checked value to the overwrite policy.
 * @return   the overwrite policy
 */
CodeGenerationPolicy::OverwritePolicy CodeGenOptionsPage::widgetToOverwrite()
{
    if (ui_radioButtonOverwrite->isChecked()) {
        return CodeGenerationPolicy::Ok;
    }
    if (ui_radioButtonAsk->isChecked()) {
        return CodeGenerationPolicy::Ask;
    }
    if (ui_radioButtonChangeName->isChecked()) {
        return CodeGenerationPolicy::Never;
    }
    return CodeGenerationPolicy::Ask;
}

/**
 * Updates the code generation policy tab.
 */
void CodeGenOptionsPage::updateCodeGenerationPolicyTab()
{
    if (m_pCodePolicyPage)
    {
        ui_tabWidgetMain->removeTab(2);
        m_pCodePolicyPage->disconnect();

        delete m_pCodePolicyPage;
        m_pCodePolicyPage = 0;
    }

    Uml::ProgrammingLanguage pl = Uml::ProgrammingLanguage::Value(ui_SelectLanguageBox->currentIndex());
    CodeGenPolicyExt *policyExt = CodeGenFactory::newCodeGenPolicyExt(pl);

    if (policyExt) {
        m_pCodePolicyPage = policyExt->createPage(0 , "codelangpolicypage");
    }
    else {
        m_pCodePolicyPage = new DefaultCodeGenPolicyPage(0 , "codelangpolicypage");
    }

    ui_tabWidgetMain->insertTab(2, m_pCodePolicyPage, i18n("Language Options" ) );

    connect(this, SIGNAL(applyClicked()), m_pCodePolicyPage, SLOT(apply()));
}

/**
 * Reimplemented QWizardPage method to validate page when user clicks next button.
 * @return   the validation state
 */
bool CodeGenOptionsPage::validatePage()
{
    return save();
}

/**
 * Reads the set values from their corresponding widgets, writes them back to 
 * the data structure, and notifies clients.
 */
void CodeGenOptionsPage::apply()
{
    if (m_parentPolicy) {
        m_parentPolicy->setCodeVerboseDocumentComments(ui_forceDoc->isChecked());
        m_parentPolicy->setCodeVerboseSectionComments(ui_forceSections->isChecked());
        m_parentPolicy->setOutputDirectory(QDir(ui_outputDir->text()));
        m_parentPolicy->setIncludeHeadings(ui_includeHeadings->isChecked());
        m_parentPolicy->setHeadingFileDir(ui_headingsDir->text());
        m_parentPolicy->setOverwritePolicy(widgetToOverwrite());
        m_parentPolicy->setLineEndingType((CodeGenerationPolicy::NewLineType) ui_SelectEndLineCharsBox->currentIndex());
        m_parentPolicy->setIndentationType((CodeGenerationPolicy::IndentationType) ui_SelectIndentationTypeBox->currentIndex());
        m_parentPolicy->setIndentationAmount(ui_SelectIndentationNumber->value());

        m_pCodePolicyPage->apply();
        // Emit in THIS order. The first signal triggers any sub-class to do its apply
        // slot, THEN, once we are all updated, we may sync the parent generator's code
        // documents.
        emit applyClicked();
        emit syncCodeDocumentsToParent();
    }
}

/**
 * This function is called when leaving this wizard page.
 * Saves the made settings and checks some values.
 * @return   the success state
 */
bool CodeGenOptionsPage::save()
{
    // first save the settings to the selected generator policy
    apply();

    // before going on to the generation page, check that the output directory
    // exists and is writable

    // get the policy for the current code generator
    CodeGenerationPolicy *policy = UMLApp::app()->commonPolicy();

    // get the output directory path
    QFileInfo info(policy->getOutputDirectory().absolutePath());
    if (info.exists()) {
        // directory exists... make sure we can write to it
        if (!info.isWritable()) {
            KMessageBox::sorry(this,i18n("The output folder exists, but it is not writable.\nPlease set the appropriate permissions or choose another folder."),
                    i18n("Error Writing to Output Folder"));
            return false;
        }
        // it exits and we can write... make sure it is a directory
        if (!info.isDir()) {
            KMessageBox::sorry(this,i18n("%1 does not seem to be a folder. Please choose a valid folder.", info.filePath()),
                    i18n("Please Choose Valid Folder"));
            return false;
        }
    }
    else {
        if (KMessageBox::questionYesNo(this,
                        i18n("The folder %1 does not exist. Do you want to create it now?", info.filePath()),
                        i18n("Output Folder Does Not Exist"), KGuiItem(i18n("Create Folder")), KGuiItem(i18n("Do Not Create"))) == KMessageBox::Yes)
        {
            QDir dir;
            if (!dir.mkdir(info.filePath())) {
                KMessageBox::sorry(this,i18n("The folder could not be created.\nPlease make sure you have write access to its parent folder or select another, valid, folder."),
                            i18n("Error Creating Folder"));
                return false;
            }
            // else, directory created
        }
        else {  // do not create output directory
            KMessageBox::information(this,i18n("Please select a valid folder."),
                          i18n("Output Folder Does Not Exist"));
            return false;
        }
    }
    return true;
}

/**
 * Transform signal.
 * @param id   position in combo box
 */
void CodeGenOptionsPage::activeLanguageChanged(int id)
{
    Q_UNUSED(id);
    emit languageChanged();
}

/**
 * When the user changes the language, the codegenoptions page
 * language-dependent stuff has to be updated.
 * The way to do this is to call its "apply" method.
 */
void CodeGenOptionsPage::changeLanguage()
{
    QString plStr = getLanguage();
    Uml::ProgrammingLanguage pl = Uml::ProgrammingLanguage::fromString(plStr);
    UMLApp::app()->setActiveLanguage(pl);
    /* @todo is this needed? if yes adapt to new scheme
     m_CodeGenOptionsPage->setCodeGenerator(m_doc->getCurrentCodeGenerator());
     */
    apply();
}

/**
 * Slot for clicked events of the browse buttons.
 * The selected directory is written to its corresponding text field.
 */
void CodeGenOptionsPage::browseClicked()
{
    QString button = sender()->objectName();
    QString dir = KFileDialog::getExistingDirectory();
    if (dir.isEmpty()) {
        return;
    }
    if (button == "ui_browseOutput") {
        ui_outputDir->setText(dir);
    }
    else if (button == "ui_browseHeadings") {
        ui_headingsDir->setText(dir);
    }
}

/**
 * Returns the user selected language used for code generation.
 * @return   the programming language name 
 */
QString CodeGenOptionsPage::getLanguage()
{
    return ui_SelectLanguageBox->currentText();
}

#include "codegenoptionspage.moc"

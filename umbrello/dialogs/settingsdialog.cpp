/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "settingsdialog.h"

// app includes
#include "autolayoutoptionpage.h"
#include "classoptionspage.h"
#include "codeimportoptionspage.h"
#include "codegenoptionspage.h"
#include "uioptionspage.h"
#include "umlwidgetstylepage.h"
#include "codevieweroptionspage.h"
#include "generaloptionpage.h"
#include "dontaskagain.h"
#include "debug_utils.h"
#include "icon_utils.h"
#include "layoutgenerator.h"
#include "umbrellosettings.h"

// kde includes
#include <KColorButton>

// qt includes
#include <QCheckBox>
#include <QFontDialog>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget * parent, Settings::OptionState *state)
  : MultiPageDialogBase(parent, true)
{
    setCaption(i18n("Umbrello Setup"));
    m_bChangesApplied = false;
    m_pOptionState = state;
    setupGeneralPage();
    m_dontAskAgainWidget = DontAskAgainHandler::instance().createWidget();
    m_pGeneralPage->layout()->addWidget(m_dontAskAgainWidget);
    pageFont = setupFontPage(state->uiState.font);
    setupUIPage();
    setupClassPage();
    setupCodeImportPage();
    setupCodeGenPage();
    setupCodeViewerPage(state->codeViewerState);
    setupAutoLayoutPage();
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
    connect(this, SIGNAL(defaultClicked()), this, SLOT(slotDefault()));
}

SettingsDialog::~SettingsDialog()
{
}

/**
 * Set current page
 *
 * @param page the page to set
 */
void SettingsDialog::setCurrentPage(PageType page)
{
    KPageWidgetItem *currentPage;

    switch(page) {
    case FontPage:
        currentPage = pageFont;
        break;
    case UserInterfacePage:
        currentPage = pageUserInterface;
        break;
    case AutoLayoutPage:
        currentPage = pageAutoLayout;
        break;
    case CodeImportPage:
        currentPage = pageCodeImport;
        break;
    case CodeGenerationPage:
        currentPage = pageCodeGen;
        break;
    case CodeViewerPage:
        currentPage = pageCodeViewer;
        break;
    case ClassPage:
        currentPage = pageClass;
        break;
    case GeneralPage:
    default:
        currentPage = pageGeneral;
        break;
    }
    MultiPageDialogBase::setCurrentPage(currentPage);
}


void SettingsDialog::setupUIPage()
{
    m_uiOptionsPage = new UIOptionsPage(nullptr, m_pOptionState);
    pageUserInterface = createPage(i18n("User Interface"), i18n("User Interface Settings"),
                                   Icon_Utils::it_Properties_UserInterface, m_uiOptionsPage);
}

void SettingsDialog::setupGeneralPage()
{
    m_pGeneralPage = new GeneralOptionPage;
    pageGeneral = createPage(i18nc("general settings page", "General"), i18n("General Settings"),
                             Icon_Utils::it_Properties_General, m_pGeneralPage);
    m_pGeneralPage->setMinimumSize(310, 330);

}

void SettingsDialog::setupClassPage()
{
    m_pClassPage = new ClassOptionsPage(nullptr, m_pOptionState, false);
    pageClass = createPage(i18nc("class settings page", "Class"), i18n("Class Settings"),
                           Icon_Utils::it_Properties_Class, m_pClassPage);
}

void SettingsDialog::setupCodeImportPage()
{
    m_pCodeImportPage = new CodeImportOptionsPage;
    pageCodeImport = createPage(i18n("Code Importer"), i18n("Code Import Settings"),
                                Icon_Utils::it_Properties_CodeImport, m_pCodeImportPage);
}

void SettingsDialog::setupCodeGenPage()
{
    m_pCodeGenPage = new CodeGenOptionsPage;
    connect(m_pCodeGenPage, SIGNAL(languageChanged()), this, SLOT(slotApply()));

    pageCodeGen = createPage(i18n("Code Generation"), i18n("Code Generation Settings"),
                             Icon_Utils::it_Properties_CodeGeneration, m_pCodeGenPage);
}

void SettingsDialog::setupCodeViewerPage(Settings::CodeViewerState options)
{
    //setup code generation settings page
    m_pCodeViewerPage = new CodeViewerOptionsPage(options);
    pageCodeViewer = createPage(i18n("Code Viewer"), i18n("Code Viewer Settings"),
                                Icon_Utils::it_Properties_CodeViewer, m_pCodeViewerPage);
}

void SettingsDialog::setupAutoLayoutPage()
{
    m_pAutoLayoutPage = new AutoLayoutOptionPage;
    pageAutoLayout = createPage(i18n("Auto Layout"), i18n("Auto Layout Settings"),
                                Icon_Utils::it_Properties_AutoLayout, m_pAutoLayoutPage);
}

void SettingsDialog::slotApply()
{
    applyPage(currentPage());
    //do not emit signal applyClicked in the slot slotApply->infinite loop
    //emit applyClicked();
}

void SettingsDialog::slotOk()
{
    applyPage(pageClass);
    applyPage(pageGeneral);
    applyPage(pageUserInterface);
    applyPage(pageCodeViewer);
    applyPage(pageCodeImport);
    applyPage(pageCodeGen);
    applyPage(pageFont);
    applyPage(pageAutoLayout);
    m_pOptionState->save();
    UmbrelloSettings::self()->save();
    accept();
}

void SettingsDialog::slotDefault()
{
    // Defaults hard coded.  Make sure that this is alright.
    // If defaults are set anywhere else, like in setting up config file, make sure the same.
    KPageWidgetItem *current = currentPage();
    if (current ==  pageGeneral)
    {
        m_pGeneralPage->setDefaults();
        m_dontAskAgainWidget->setDefaults();
    }
    else if (current == pageFont)
    {
        resetFontPage(parentWidget());
    }
    else if (current == pageUserInterface)
    {
        m_uiOptionsPage->setDefaults();
    }
    else if (current == pageClass)
    {
        m_pClassPage->setDefaults();
    }
    else if (current == pageCodeImport)
    {
        m_pCodeImportPage->setDefaults();
    }
    else if (current == pageCodeGen)
    {
    }
    else if (current == pageCodeViewer)
    {
    }
    else if (current == pageAutoLayout)
    {
        m_pAutoLayoutPage->setDefaults();
    }
}

void SettingsDialog::applyPage(KPageWidgetItem*item)
{
    m_bChangesApplied = true;
    if (item == pageGeneral)
    {
        m_pGeneralPage->apply();
        m_dontAskAgainWidget->apply();
    }
    else if (item == pageFont)
    {
        applyFontPage(m_pOptionState);
    }
    else if (item == pageUserInterface)
    {
        m_uiOptionsPage->apply();
    }
    else if (item == pageClass)
    {
        m_pClassPage->apply();
    }
    else if (item == pageCodeImport)
    {
        m_pCodeImportPage->apply();
    }
    else if (item == pageCodeGen)
    {
        m_pCodeGenPage->apply();
    }
    else if (item == pageCodeViewer)
    {
        m_pCodeViewerPage->apply();
        m_pOptionState->codeViewerState = m_pCodeViewerPage->getOptions();
    }
    else if (item == pageAutoLayout)
    {
        m_pAutoLayoutPage->apply();
    }
}

QString SettingsDialog::getCodeGenerationLanguage()
{
    return m_pCodeGenPage->getLanguage();
}


/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Paul Hensgen <phensgen@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codegenerationwizard.h"

// local includes
#include "codegenselectpage.h"
#include "codegenoptionspage.h"
#include "codegenstatuspage.h"
#include "umlclassifier.h"
#include "icon_utils.h"
#include "umlapp.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QWizardPage>

/**
 * Constructor. Sets up the wizard and loads the wizard pages.
 * Each wizard page has its own class.
 * @param classList   the list of classes, which have to be generated
 */
CodeGenerationWizard::CodeGenerationWizard(UMLClassifierList *classList)
  : QWizard((QWidget*)UMLApp::app())
{
    setWizardStyle(QWizard::ModernStyle);
    setPixmap(QWizard::LogoPixmap, Icon_Utils::DesktopIcon(Icon_Utils::it_Code_Gen_Wizard));
    setWindowTitle(i18n("Code Generation Wizard"));
    setOption(QWizard::NoBackButtonOnStartPage, true);

    setPage(OptionsPage, createOptionsPage());
    setPage(SelectionPage, createSelectionPage(classList));
    setPage(StatusPage, createStatusPage());
    connect(m_OptionsPage, SIGNAL(languageChanged()), this, SLOT(slotLanguageChanged()));
}

/**
 * Destructor.
 */
CodeGenerationWizard::~CodeGenerationWizard()
{
}

/**
 * Creates the class selection page.
 * @param classList   the list of classes, which have to be generated
 * @return            the wizard page
 */
QWizardPage* CodeGenerationWizard::createSelectionPage(UMLClassifierList *classList)
{
    m_SelectionPage = new CodeGenSelectPage(this);
    m_SelectionPage->setClassifierList(classList);
    return m_SelectionPage;
}

/**
 * Creates the code generation options page, which allows to tune 
 * the code generation by setting some parameters.
 * @return   the wizard page
 */
QWizardPage* CodeGenerationWizard::createOptionsPage()
{
    m_OptionsPage = new CodeGenOptionsPage(this);
    return m_OptionsPage;
}

/**
 * Creates the code generation status page, which shows the progress
 * of the generation.
 * @return   the wizard page
 */
QWizardPage* CodeGenerationWizard::createStatusPage()
{
    m_StatusPage = new CodeGenStatusPage(this);
    return m_StatusPage;
}

/**
 * Returns the list widget, which holds the classes for generation.
 * With this function the list of classes to generate can be transferred
 * from the select page to the status page.
 * @return   the list widget
 */
QListWidget* CodeGenerationWizard::getSelectionListWidget()
{
    return m_SelectionPage->getSelectionListWidget();
}

/**
 * Slot to handle generator language change.
 */
void CodeGenerationWizard::slotLanguageChanged()
{
    m_SelectionPage->setClassifierList(nullptr);
}

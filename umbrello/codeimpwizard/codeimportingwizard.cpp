/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "codeimportingwizard.h"

// local includes
#include "codeimpselectpage.h"
#include "codeimpstatuspage.h"
#include "classifier.h"
#include "icon_utils.h"
#include "uml.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QFileInfo>
#include <QWizardPage>

/**
 * Constructor. Sets up the wizard but does not load the wizard pages.
 * Each wizard page has its own class.
 * Loading of the wizard pages needs to be done in separate step because
 * it is too early to do that within the constructor (`this` not finalized).
 * See https://bugs.kde.org/show_bug.cgi?id=479224
 */
CodeImportingWizard::CodeImportingWizard()
  : QWizard((QWidget*)UMLApp::app())
{
    setWizardStyle(QWizard::ModernStyle);
    setPixmap(QWizard::LogoPixmap, Icon_Utils::DesktopIcon(Icon_Utils::it_Code_Gen_Wizard));
    setWindowTitle(i18n("Code Importing Wizard"));
    setOption(QWizard::NoBackButtonOnStartPage, true);
}

/**
 * Destructor.
 */
CodeImportingWizard::~CodeImportingWizard()
{
}

/**
 * Set up the SelectionPage and the StatusPage.
 * This needs to be called after constructing the CodeImportingWizard.
 */
void CodeImportingWizard::setupPages()
{
    setPage(SelectionPage, createSelectionPage());
    setPage(StatusPage, createStatusPage());
}

/**
 * Creates the class selection page.
 * @param classList   the list of classes, which have to be imported
 * @return            the wizard page
 */
QWizardPage* CodeImportingWizard::createSelectionPage()
{
    m_SelectionPage = new CodeImpSelectPage(this);
    return m_SelectionPage;
}

/**
 * Creates the code importing status page, which shows the progress
 * of the import process.
 * @return   the wizard page
 */
QWizardPage* CodeImportingWizard::createStatusPage()
{
    m_StatusPage = new CodeImpStatusPage(this);
    return m_StatusPage;
}

/**
 * Returns a list, which contains the classes for importing.
 * With this function the list of classes to import can be transferred
 * from the select page to the status page.
 * @return   the file info list
 */
QList<QFileInfo> CodeImportingWizard::selectedFiles()
{
    return m_SelectionPage->selectedFiles();
}

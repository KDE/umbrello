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
#if QT_VERSION >= 0x050000
#else
#include <kdebug.h>
#endif
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QFileInfo>
#include <QWizardPage>

/**
 * Constructor. Sets up the wizard and loads the wizard pages.
 * Each wizard page has its own class.
 */
CodeImportingWizard::CodeImportingWizard()
  : QWizard((QWidget*)UMLApp::app())
{
    setWizardStyle(QWizard::ModernStyle);
    setPixmap(QWizard::LogoPixmap, Icon_Utils::DesktopIcon(Icon_Utils::it_Code_Gen_Wizard));
    setWindowTitle(i18n("Code Importing Wizard"));
    setOption(QWizard::NoBackButtonOnStartPage, true);

    setPage(SelectionPage, createSelectionPage());
    setPage(StatusPage, createStatusPage());
}

/**
 * Destructor.
 */
CodeImportingWizard::~CodeImportingWizard()
{
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

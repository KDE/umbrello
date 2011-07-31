/*
    Copyright 2011  Andi Fischer  <andi.fischer@hispeed.ch>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "codeimportingwizard.h"

// local includes
#include "codeimpselectpage.h"
#include "codeimpstatuspage.h"
#include "classifier.h"
#include "icon_utils.h"
#include "uml.h"

// kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

// qt includes
#include <QtCore/QFileInfo>
#include <QtGui/QWizardPage>

/**
 * Constant value for the fixed size of the wizard pages.
 */
const QSize CodeImportingWizard::c_pageSize = QSize(520, 530);

/**
 * Constructor. Sets up the wizard and loads the wizard pages.
 * Each wizard page has its own class.
 * @param classList   the list of classes, which have to be generated
 */
CodeImportingWizard::CodeImportingWizard()
  : QWizard((QWidget*)UMLApp::app())
{
    setWizardStyle(QWizard::ModernStyle);
    setPixmap(QWizard::LogoPixmap, Icon_Utils::UserIcon(Icon_Utils::it_Code_Gen_Wizard));
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
    m_SelectionPage->setFixedSize(c_pageSize);
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
    m_StatusPage->setFixedSize(c_pageSize);
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

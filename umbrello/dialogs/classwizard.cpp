/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "classwizard.h"

// local includes
#include "attribute.h"
#include "classifier.h"
#include "classifierlistitem.h"
#include "classifierlistpage.h"
#include "classgeneralpage.h"
#include "operation.h"
#include "uml.h"
#include "umldoc.h"
#include "umlclassifierlistitemlist.h"

// kde includes
#include <khelpmenu.h>
#include <KLocalizedString>

// qt includes
#include <QVBoxLayout>
#include <QWizardPage>

/**
 * Constructor. Sets up the wizard and loads the wizard pages.
 * Each wizard page has its own class.
 * @param doc   the UML document
 */
ClassWizard::ClassWizard(UMLDoc* doc)
    : QWizard((QWidget*)doc->parent())
{
    m_doc = doc;
    //create a unique class to start with
    UMLObject * pTemp = 0;
    QString name = i18n("new_class");
    QString newName = name;
    QString num;
    int i = 0;
    m_pClass = new UMLClassifier(newName);
    do {
        m_pClass->setName(newName);
        pTemp = m_doc->findUMLObject(newName);
        num.setNum(++i);
        newName = name;
        newName.append(QChar::fromLatin1('_')).append(num);
    } while(pTemp);

    setWizardStyle(QWizard::ModernStyle);
    setPixmap(QWizard::LogoPixmap, Icon_Utils::DesktopIcon(Icon_Utils::it_Code_Gen_Wizard));
    setWindowTitle(i18n("Class Wizard"));
    setOption(QWizard::NoBackButtonOnStartPage, true);
    setOption(QWizard::HaveHelpButton, true);
    connect(this, SIGNAL(helpRequested()), this, SLOT(showHelp()));

    addPage(createGeneralPage());
    addPage(createAttributesPage());
    addPage(createOperationsPage());
}

/**
 * Destructor.
 */
ClassWizard::~ClassWizard()
{
}

/**
 * Create page 1 of wizard - the general class info.
 */
QWizardPage* ClassWizard::createGeneralPage()
{
    m_GeneralPage = new QWizardPage;
    m_GeneralPage->setTitle(i18n("New Class"));
    m_GeneralPage->setSubTitle(i18n("Add general info about the new class."));

    m_pGenPage = new ClassGeneralPage(m_doc, this, m_pClass);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_pGenPage);
    m_GeneralPage->setLayout(layout);

    return m_GeneralPage;
}

/**
 * Create page 2 of wizard - the class attributes editor.
 */
QWizardPage* ClassWizard::createAttributesPage()
{
    m_AttributesPage = new QWizardPage;
    m_AttributesPage->setTitle(i18n("Class Attributes"));
    m_AttributesPage->setSubTitle(i18n("Add attributes to the new class."));

    m_pAttPage = new ClassifierListPage(this, m_pClass, m_doc, UMLObject::ot_Attribute);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_pAttPage);
    m_AttributesPage->setLayout(layout);

    return m_AttributesPage;
}

/**
 * Create page 3 of wizard - the class operations editor.
 */
QWizardPage* ClassWizard::createOperationsPage()
{
    m_OperationsPage = new QWizardPage;
    m_OperationsPage->setTitle(i18n("Class Operations"));
    m_OperationsPage->setSubTitle(i18n("Add operations to the new class."));

    m_pOpPage = new ClassifierListPage(this, m_pClass, m_doc, UMLObject::ot_Operation);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_pOpPage);
    m_OperationsPage->setLayout(layout);

    return m_OperationsPage;
}

/**
 * Advances to the next page. Is called when the next button is pressed.
 */
void ClassWizard::next()
{
    QWizardPage* page = currentPage();
    if (page == m_GeneralPage) {
        m_pGenPage->apply();
    } else if (page == m_AttributesPage) {
        m_pAttPage->apply();
    }
    QWizard::next();
}

/**
 * Back button was called.
 */
void ClassWizard::back()
{
    QWizardPage* page = currentPage();
    if (page == m_AttributesPage) {
        m_pAttPage->apply();
    } else if (page == m_OperationsPage) {
        m_pOpPage->apply();
    }
    QWizard::back();
}

/**
 * Finish button was called.
 * @todo Calling m_pGenPage->apply() twice is ugly,
 *       but without the first call the documentation of the class is cleared.
 */
void ClassWizard::accept()
{
    m_pGenPage->apply();

    m_doc->addUMLObject(m_pClass);
    m_doc->signalUMLObjectCreated(m_pClass);

    // call apply of General Page again so as to bind to package
    // now that the classifier object is in the document.
    m_pGenPage->apply();

    QWizard::accept();
}

/**
 * Cancel button was called.
 */
void ClassWizard::reject()
{
    m_doc->removeUMLObject(m_pClass, true);
    QWizard::reject();
}

/**
 * Opens Umbrello handbook. Is called when help button is pressed.
 */
void ClassWizard::showHelp()
{
    KHelpMenu helpMenu(this);
    helpMenu.appHelpActivated();
}

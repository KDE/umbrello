/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "classpropertiesdialog.h"

// app includes
#include "classassociationspage.h"
#include "classgeneralpage.h"
#include "classifierlistpage.h"
#include "classifierwidget.h"
#include "classoptionspage.h"
#include "componentwidget.h"
#include "constraintlistpage.h"
#include "debug_utils.h"
#include "umlentity.h"
#include "objectwidget.h"
#include "packagecontentspage.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlview.h"
#include "umlwidgetstylepage.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QFrame>
#include <QHBoxLayout>

/**
 *  Sets up a ClassPropDialog.
 *
 *  @param parent    The parent of the ClassPropDialog
 *  @param c         The UMLObject to display properties of.
 *  @param assoc     Determines whether to display associations
 */
ClassPropertiesDialog::ClassPropertiesDialog(QWidget *parent, UMLObject * c, bool assoc)
  : MultiPageDialogBase(parent)
{
    init();
    m_pWidget = nullptr;
    m_pObject = c;

    setupPages(assoc);

    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

/**
 *  Sets up a ClassPropDialog.
 *
 *  @param  parent  The parent of the ClassPropDialog
 *  @param  o       The ObjectWidget to display properties of.
 */
ClassPropertiesDialog::ClassPropertiesDialog(QWidget *parent, ObjectWidget *o)
  : MultiPageDialogBase(parent)
{
    init();
    m_pWidget = o;
    m_pObject = m_pWidget->umlObject();
    m_doc = UMLApp::app()->document();

    setupGeneralPage();
    setupStylePage(m_pWidget);
    setupFontPage(m_pWidget);

    setMinimumSize(340, 420);
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

/**
 *  Sets up a ClassPropDialog.
 *
 *  @param  parent  The parent of the ClassPropDialog
 *  @param  w       The UMLWidget to display properties of.
 */
ClassPropertiesDialog::ClassPropertiesDialog(QWidget *parent, UMLWidget *w)
  : MultiPageDialogBase(parent)
{
    init();
    m_pWidget = w;
    m_pObject = w->umlObject();

    if (w->isClassWidget()
            || w->isInterfaceWidget()
            || w->isEnumWidget()
            || w->isPackageWidget()) {
        setupPages(true);
    } else if (w->isComponentWidget()) {
        if (w->isInstance()) {
            setupInstancePages();
        } else {
            setupPages(true);
        }
    } else if (w->isNodeWidget()) {
        setupInstancePages();
    } else {
        setupPages();
    }

    // now setup the options page for classes
    if (w->isClassWidget() || w->isInterfaceWidget()) {
        setupDisplayPage();
    }
    setupStylePage(m_pWidget);
    setupFontPage(m_pWidget);
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

void ClassPropertiesDialog::init()
{
    setCaption(i18n("Properties"));
    m_pAssocPage = nullptr;
    m_pGenPage = nullptr;
    m_pAttPage = nullptr;
    m_pOpsPage = nullptr;
    m_pPkgContentsPage = nullptr;
    m_pTemplatePage = nullptr;
    m_pEnumLiteralPage = nullptr;
    m_pEntityAttributePage = nullptr;
    m_pEntityConstraintPage = nullptr;
    m_pOptionsPage = nullptr;
    m_doc = UMLApp::app()->document();
}

/**
 *  Standard destructor.
 */
ClassPropertiesDialog::~ClassPropertiesDialog()
{
}

/**
 * Calls slotApply()
 */
void ClassPropertiesDialog::apply()
{
    slotApply();
}

/**
 * Calls slotApply() and accepts (closes) the dialog.
 */
void ClassPropertiesDialog::slotOk()
{
    slotApply();
    accept();
}

/**
 * Applies the settings in the dialog to the widget and object.
 */
void ClassPropertiesDialog::slotApply()
{
    if (m_pGenPage) {
        m_pGenPage->apply();
    }
    if (m_pAttPage) {
        m_pAttPage->apply();
    }
    if (m_pOpsPage) {
        m_pOpsPage->apply();
    }
    if (m_pTemplatePage) {
        m_pTemplatePage->apply();
    }
    if (m_pEnumLiteralPage) {
        m_pEnumLiteralPage->apply();
    }
    if (m_pEntityAttributePage) {
        m_pEntityAttributePage->apply();
    }
    if (m_pEntityConstraintPage) {
        m_pEntityConstraintPage->apply();
    }
    if (m_pOptionsPage) {
        m_pOptionsPage->apply();
    }
    if (m_pStylePage) {
        m_pStylePage->apply();
    }
    if (m_pWidget) {
        applyFontPage(m_pWidget);
    }
}

/**
 * Sets up the general, attribute, operations, template and association pages as appropriate.
 */
void ClassPropertiesDialog::setupPages(bool assoc)
{
    setupGeneralPage();

    UMLObject::ObjectType ot = UMLObject::ot_UMLObject;
    if (m_pObject) {
        ot = m_pObject->baseType();
    }
    // add extra pages for class
    if (ot == UMLObject::ot_Class) {
        setupAttributesPage();
    }
    if (ot == UMLObject::ot_Class || ot == UMLObject::ot_Interface) {
        setupOperationsPage();
    }
    if (ot == UMLObject::ot_Class || ot == UMLObject::ot_Interface) {
        setupTemplatesPage();
    }
    if (ot == UMLObject::ot_Enum) {
        setupEnumLiteralsPage();
    }
    if (ot == UMLObject::ot_Entity) {
        setupEntityAttributesPage();
        setupEntityConstraintsPage();
        if (m_pWidget && m_pWidget->isEntityWidget())
            setupEntityDisplayPage(m_pWidget->asEntityWidget());
    }
    if (ot == UMLObject::ot_Package) {
        setupContentsPage();
    }
    if (assoc) {
        setupAssociationsPage();
    } else {
        m_pAssocPage = nullptr;
    }
}

/**
 * Sets up the page "General" for the component.
 */
void ClassPropertiesDialog::setupGeneralPage()
{
    if (m_pWidget && m_pWidget->baseType() == UMLWidget::wt_Object)
        m_pGenPage = new ClassGeneralPage(m_doc, nullptr, static_cast<ObjectWidget*>(m_pWidget));
    else if (m_pWidget && !m_pObject)
        m_pGenPage = new ClassGeneralPage(m_doc, nullptr, m_pWidget);
    else
        m_pGenPage = new ClassGeneralPage(m_doc, nullptr, m_pObject);
    createPage(i18nc("general settings page name", "General"), i18n("General Settings"),
               Icon_Utils::it_Properties_General, m_pGenPage)->widget()->setMinimumSize(310, 330);
    m_pGenPage->setFocus();
}

/**
 * Sets up the page "Display" for the component.
 */
void ClassPropertiesDialog::setupDisplayPage()
{
    ClassifierWidget *cw = m_pWidget->asClassifierWidget();
    m_pOptionsPage = new ClassOptionsPage(nullptr, cw);
    createPage(i18nc("display option page name", "Display"), i18n("Display Options"),
               Icon_Utils::it_Properties_Display, m_pOptionsPage);
}

/**
 * Sets up the page "Display" for the component.
 */
void ClassPropertiesDialog::setupEntityDisplayPage(EntityWidget *widget)
{
    m_pOptionsPage = new ClassOptionsPage(nullptr, widget);
    createPage(i18nc("display option page name", "Display"), i18n("Display Options"),
               Icon_Utils::it_Properties_Display, m_pOptionsPage);
}

/**
 * Sets up the page "Attributes" for the component.
 */
void ClassPropertiesDialog::setupAttributesPage()
{
    m_pAttPage = new ClassifierListPage(nullptr, (UMLClassifier *)m_pObject, m_doc, UMLObject::ot_Attribute);
    createPage(i18n("Attributes"), i18n("Attribute Settings"),
               Icon_Utils::it_Properties_Attributes, m_pAttPage);
}

/**
 * Sets up the page "Operations" for the component.
 */
void ClassPropertiesDialog::setupOperationsPage()
{
    m_pOpsPage = new ClassifierListPage(nullptr, (UMLClassifier*)m_pObject, m_doc, UMLObject::ot_Operation);
    createPage(i18n("Operations"), i18n("Operation Settings"),
               Icon_Utils::it_Properties_Operations, m_pOpsPage);
}

/**
 * Sets up the page "Templates" for the component.
 */
void ClassPropertiesDialog::setupTemplatesPage()
{
    m_pTemplatePage = new ClassifierListPage(nullptr, (UMLClassifier *)m_pObject, m_doc, UMLObject::ot_Template);
    createPage(i18n("Templates"), i18n("Templates Settings"),
               Icon_Utils::it_Properties_Templates, m_pTemplatePage);
}

/**
 * Sets up the page "Enum Literals" for the component.
 */
void ClassPropertiesDialog::setupEnumLiteralsPage()
{
    m_pEnumLiteralPage = new ClassifierListPage(nullptr, (UMLClassifier*)m_pObject, m_doc, UMLObject::ot_EnumLiteral);
    createPage(i18n("Enum Literals"), i18n("Enum Literals Settings"),
               Icon_Utils::it_Properties_EnumLiterals, m_pEnumLiteralPage);
}

/**
 * Sets up the page "Entity Attributes" for the component.
 */
void ClassPropertiesDialog::setupEntityAttributesPage()
{
    m_pEntityAttributePage = new ClassifierListPage(nullptr, (UMLEntity*)m_pObject, m_doc, UMLObject::ot_EntityAttribute);
    createPage(i18n("Entity Attributes"), i18n("Entity Attributes Settings"),
               Icon_Utils::it_Properties_EntityAttributes, m_pEntityAttributePage);
}

/**
 * Sets up the page "Entity Constraints" for the component.
 */
void ClassPropertiesDialog::setupEntityConstraintsPage()
{
    m_pEntityConstraintPage = new ConstraintListPage(nullptr, (UMLClassifier*)m_pObject, m_doc, UMLObject::ot_EntityConstraint);
    createPage(i18n("Entity Constraints"), i18n("Entity Constraints Settings"),
               Icon_Utils::it_Properties_EntityConstraints, m_pEntityConstraintPage);
}

/**
 * Sets up the page "Contents" for the component.
 */
void ClassPropertiesDialog::setupContentsPage()
{
    m_pPkgContentsPage = new PackageContentsPage(nullptr, (UMLPackage*)m_pObject);
    createPage(i18nc("contents settings page name", "Contents"), i18n("Contents Settings"),
               Icon_Utils::it_Properties_Contents, m_pPkgContentsPage);
}

/**
 * Sets up the page "Associations" for the component.
 */
void ClassPropertiesDialog::setupAssociationsPage()
{
    m_pAssocPage = new ClassAssociationsPage(nullptr, UMLApp::app()->currentView()->umlScene(), m_pObject);
    createPage(i18n("Associations"), i18n("Class Associations"),
               Icon_Utils::it_Properties_Associations, m_pAssocPage);
}

/**
 * Sets up the general page for the component.
 */
void ClassPropertiesDialog::setupInstancePages()
{
    m_pGenPage = new ClassGeneralPage(m_doc, nullptr, m_pWidget);
    createPage(i18nc("instance general settings page name", "General"), i18n("General Settings"),
               Icon_Utils::it_Properties_General, m_pGenPage)->widget()->setMinimumSize(310, 330);
    m_pAssocPage = nullptr;
}


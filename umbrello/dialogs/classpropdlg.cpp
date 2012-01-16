/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "classpropdlg.h"

// app includes
#include "assocpage.h"
#include "classgenpage.h"
#include "classifierlistpage.h"
#include "classifierwidget.h"
#include "classoptionspage.h"
#include "componentwidget.h"
#include "constraintlistpage.h"
#include "entity.h"
#include "objectwidget.h"
#include "pkgcontentspage.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlview.h"
#include "umlwidgetstylepage.h"

// kde includes
#include <klocale.h>
#include <kvbox.h>

// qt includes
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>


/**
 *  Sets up a ClassPropDlg.
 *
 *  @param parent    The parent of the ClassPropDlg
 *  @param c         The UMLObject to display properties of.
 *  @param assoc     Determines whether to display associations
 */
ClassPropDlg::ClassPropDlg(QWidget *parent, UMLObject * c, bool assoc)
  : DialogBase(parent)
{
    init();
    m_pWidget = 0;
    m_Type = pt_Object;
    m_pObject = c;

    setupPages(assoc);

    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

/**
 *  Sets up a ClassPropDlg.
 *
 *  @param  parent  The parent of the ClassPropDlg
 *  @param  o       The ObjectWidget to display properties of.
 */
ClassPropDlg::ClassPropDlg(QWidget *parent, ObjectWidget *o)
  : DialogBase(parent)
{
    init();
    m_pWidget = o;
    m_Type = pt_ObjectWidget;
    m_pObject = m_pWidget->umlObject();
    m_doc = UMLApp::app()->document();

    setupGeneralPage();
    setupStylePage();
    setupFontPage();

    setMinimumSize(340,420);
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

/**
 *  Sets up a ClassPropDlg.
 *
 *  @param  parent  The parent of the ClassPropDlg
 *  @param  o       The UMLWidget to display properties of.
 */
ClassPropDlg::ClassPropDlg(QWidget *parent, UMLWidget *w)
  : DialogBase(parent)
{
    init();
    m_pWidget = w;
    m_Type = pt_Widget;
    m_pObject = w->umlObject();

    if (w->baseType() == WidgetBase::wt_Class
            || w->baseType() == WidgetBase::wt_Interface
            || w->baseType() == WidgetBase::wt_Package) {
        setupPages(true);
    } else if (w->baseType() == WidgetBase::wt_Component) {
        if ( w->isInstance() ) {
            setupInstancePages();
        } else {
            setupPages();
        }
    } else if (w->baseType() == WidgetBase::wt_Node) {
        setupInstancePages();
    } else {
        setupPages();
    }

    // now setup the options page for classes
    if (w->baseType() == WidgetBase::wt_Class ||
        w->baseType() == WidgetBase::wt_Interface) {
        setupDisplayPage();
    }
    setupStylePage();
    setupFontPage();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

void ClassPropDlg::init()
{
    setCaption( i18n("Properties") );
    setButtons( Ok | Apply | Cancel | Help );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );
    setFaceType( KPageDialog::List );
    m_pGenPage = 0;
    m_pAttPage = 0;
    m_pOpsPage = 0;
    m_pTemplatePage = 0;
    m_pEnumLiteralPage = 0;
    m_pEntityAttributePage = 0;
    m_pEntityConstraintPage = 0;
    m_pOptionsPage = 0;
    m_pStylePage = 0;
    m_doc = UMLApp::app()->document();
}

/**
 *  Standard destructor.
 */
ClassPropDlg::~ClassPropDlg()
{
}

/**
 * Calls slotApply() and accepts (closes) the dialog.
 */
void ClassPropDlg::slotOk()
{
    slotApply();
    KDialog::accept();
}

/**
 * Applies the settings in the dialog to the widget and object.
 */
void ClassPropDlg::slotApply()
{
    if (m_pGenPage) {
        m_pGenPage->updateObject();
    }
    if (m_pAttPage) {
        m_pAttPage->updateObject();
    }
    if (m_pOpsPage) {
        m_pOpsPage->updateObject();
    }
    if (m_pTemplatePage) {
        m_pTemplatePage->updateObject();
    }
    if (m_pEnumLiteralPage) {
        m_pEnumLiteralPage->updateObject();
    }
    if (m_pEntityAttributePage) {
        m_pEntityAttributePage->updateObject();
    }
    if (m_pEntityConstraintPage) {
        m_pEntityConstraintPage->updateObject();
    }
    if (m_pOptionsPage) {
        m_pOptionsPage->updateUMLWidget();
    }
    if (m_pStylePage) {
        m_pStylePage->updateUMLWidget();
    }
    if (m_pWidget) {
        m_pWidget->setFont( m_pChooser->font() );
    }
}

/**
 * Sets up the general, attribute, operations, template and association pages as appropriate.
 */
void ClassPropDlg::setupPages(bool assoc)
{
    setupGeneralPage();

    UMLObject::ObjectType ot = m_pObject->baseType();
    // add extra pages for class
    if (ot == UMLObject::ot_Class ) {
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
    }
    if (ot == UMLObject::ot_Package ) {
        setupContentsPage();
    }
    if (assoc) {
        setupAssociationsPage();
    } else {
        m_pAssocPage = 0;
    }
}

/**
 * Sets up the page "General" for the component.
 */
void ClassPropDlg::setupGeneralPage()
{
    QFrame* page = createPage( i18nc("general settings page name", "General"), i18n("General Settings"),
                               Icon_Utils::it_Properties_General );
    page->setMinimumSize(310, 330);
    QHBoxLayout * topLayout = new QHBoxLayout(page);
    m_pGenPage = new ClassGenPage(m_doc, page, m_pObject);
    topLayout->addWidget(m_pGenPage);
}

/**
 * Sets up the page "Style" for the component.
 */
void ClassPropDlg::setupStylePage()
{
    QFrame * page = createPage( i18nc("widget style page name", "Style"), i18n("Widget Style"),
                                Icon_Utils::it_Properties_Color );
    QHBoxLayout * m_pStyleLayout = new QHBoxLayout(page);
    m_pStylePage = new UMLWidgetStylePage(page, m_pWidget);
    m_pStyleLayout->addWidget(m_pStylePage);
}

/**
 * Sets up the page "Display" for the component.
 */
void ClassPropDlg::setupDisplayPage()
{
    QFrame* page = createPage( i18nc("display option page name", "Display"), i18n("Display Options"),
                               Icon_Utils::it_Properties_Display );
    QHBoxLayout* m_pOptionsLayout = new QHBoxLayout(page);
    ClassifierWidget *cw = static_cast<ClassifierWidget*>(m_pWidget);
    m_pOptionsPage = new ClassOptionsPage(page, cw);
    m_pOptionsLayout->addWidget(m_pOptionsPage);
}

/**
 * Sets up the page "Attributes" for the component.
 */
void ClassPropDlg::setupAttributesPage()
{
    QFrame* page = createPage( i18n("Attributes"), i18n("Attribute Settings"),
                               Icon_Utils::it_Properties_Attributes );
    m_pAttPage = new ClassifierListPage(page, (UMLClassifier *)m_pObject, m_doc, UMLObject::ot_Attribute);
    QHBoxLayout * attLayout = new QHBoxLayout(page);
    attLayout->addWidget(m_pAttPage);
}

/**
 * Sets up the page "Operations" for the component.
 */
void ClassPropDlg::setupOperationsPage()
{
    QFrame* page = createPage( i18n("Operations"), i18n("Operation Settings"),
                               Icon_Utils::it_Properties_Operations );
    m_pOpsPage = new ClassifierListPage(page, (UMLClassifier*)m_pObject, m_doc, UMLObject::ot_Operation);
    QHBoxLayout* pOpsLayout = new QHBoxLayout(page);
    pOpsLayout->addWidget(m_pOpsPage);
}

/**
 * Sets up the page "Templates" for the component.
 */
void ClassPropDlg::setupTemplatesPage()
{
    QFrame* page = createPage( i18n("Templates"), i18n("Templates Settings"),
                               Icon_Utils::it_Properties_Templates );
    m_pTemplatePage = new ClassifierListPage(page, (UMLClassifier *)m_pObject, m_doc, UMLObject::ot_Template);
    QHBoxLayout* templatesLayout = new QHBoxLayout(page);
    templatesLayout->addWidget(m_pTemplatePage);
}

/**
 * Sets up the page "Enum Literals" for the component.
 */
void ClassPropDlg::setupEnumLiteralsPage()
{
    QFrame* page = createPage( i18n("Enum Literals"), i18n("Enum Literals Settings"),
                               Icon_Utils::it_Properties_EnumLiterals );
    m_pEnumLiteralPage = new ClassifierListPage(page, (UMLClassifier*)m_pObject, m_doc, UMLObject::ot_EnumLiteral);
    QHBoxLayout* enumLiteralsLayout = new QHBoxLayout(page);
    enumLiteralsLayout->addWidget(m_pEnumLiteralPage);
}

/**
 * Sets up the page "Entity Attributes" for the component.
 */
void ClassPropDlg::setupEntityAttributesPage()
{
    QFrame* page = createPage( i18n("Entity Attributes"), i18n("Entity Attributes Settings"),
                               Icon_Utils::it_Properties_EntityAttributes );
    m_pEntityAttributePage = new ClassifierListPage(page, (UMLEntity*)m_pObject, m_doc, UMLObject::ot_EntityAttribute);
    QHBoxLayout* entityAttributesLayout = new QHBoxLayout(page);
    entityAttributesLayout->addWidget(m_pEntityAttributePage);
}

/**
 * Sets up the page "Entity Constraints" for the component.
 */
void ClassPropDlg::setupEntityConstraintsPage()
{
    QFrame* page = createPage( i18n("Entity Constraints"), i18n("Entity Constraints Settings"),
                               Icon_Utils::it_Properties_EntityConstraints );
    m_pEntityConstraintPage = new ConstraintListPage(page, (UMLClassifier*)m_pObject, m_doc, UMLObject::ot_EntityConstraint );
    QHBoxLayout* entityConstraintsLayout = new QHBoxLayout(page);
    entityConstraintsLayout->addWidget(m_pEntityConstraintPage);
}

/**
 * Sets up the page "Contents" for the component.
 */
void ClassPropDlg::setupContentsPage()
{
    QFrame* page = createPage( i18nc("contents settings page name", "Contents"), i18n("Contents Settings"),
                               Icon_Utils::it_Properties_Contents );
    m_pPkgContentsPage = new PkgContentsPage(page, (UMLPackage*)m_pObject);
    QHBoxLayout* contentsLayout = new QHBoxLayout(page);
    contentsLayout->addWidget(m_pPkgContentsPage);
}

/**
 * Sets up the page "Associations" for the component.
 */
void ClassPropDlg::setupAssociationsPage()
{
    QFrame* page = createPage( i18n("Associations"), i18n("Class Associations"),
                               Icon_Utils::it_Properties_Associations );
    m_pAssocPage = new AssocPage(page, UMLApp::app()->currentView(), m_pObject);
    QHBoxLayout* assocLayout = new QHBoxLayout(page);
    assocLayout->addWidget(m_pAssocPage);
}

/**
 * Sets up the general page for the component.
 */
void ClassPropDlg::setupInstancePages()
{
    QFrame* page = createPage( i18nc("instance general settings page name", "General"), i18n("General Settings"),
                               Icon_Utils::it_Properties_General );
    QHBoxLayout* genLayout = new QHBoxLayout(page);
    page->setMinimumSize(310, 330);
    m_pGenPage = new ClassGenPage(m_doc, page, m_pWidget);
    genLayout->addWidget(m_pGenPage);
    m_pAssocPage = 0;
}

/**
 * Sets up the font page.
 */
void ClassPropDlg::setupFontPage()
{
    if ( !m_pWidget ) {
        return;
    }
    KVBox* page = new KVBox();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, i18n("Font") );
    pageItem->setHeader( i18n("Font Settings") );
    pageItem->setIcon( Icon_Utils::DesktopIcon(Icon_Utils::it_Properties_Font) );
    addPage( pageItem );
    m_pChooser = new KFontChooser( (QWidget*)page, KFontChooser::NoDisplayFlags, QStringList(), 0);
    m_pChooser->setFont( m_pWidget->font() );
}

#include "classpropdlg.moc"

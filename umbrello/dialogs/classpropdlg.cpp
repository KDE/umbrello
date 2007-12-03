/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "classpropdlg.h"

// qt/kde includes
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kvbox.h>
#include <kicon.h>
// app includes
#include "classgenpage.h"
#include "classifierlistpage.h"
#include "constraintlistpage.h"
#include "pkgcontentspage.h"
#include "assocpage.h"
#include "classoptionspage.h"
#include "umlwidgetcolorpage.h"
#include "../umlobject.h"
#include "../umldoc.h"
#include "../classifierwidget.h"
#include "../objectwidget.h"
#include "../componentwidget.h"
#include "../uml.h"
#include "../umlview.h"
#include "../entity.h"

ClassPropDlg::ClassPropDlg(QWidget *parent, UMLObject * c, int /*pageNum*/, bool assoc)
        : KPageDialog(parent)
{
    init();
    m_pWidget = 0;
    m_Type = pt_Object;
    m_pObject = c;

    setupPages(assoc);

#ifdef __GNUC__
#warning "kde4: reimplement showPage"
#endif
    //showPage(pageNum);

    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

ClassPropDlg::ClassPropDlg(QWidget *parent, ObjectWidget *o)
        : KPageDialog(parent)
{
    init();
    m_pWidget = o;
    m_Type = pt_ObjectWidget;
    m_pObject = m_pWidget->getUMLObject();
    m_pDoc = UMLApp::app()->getDocument();

    setupGeneralPage();
    setupColorPage();
    setupFontPage();

#ifdef __GNUC__
#warning "kde4: reimplement showPage"
#endif
    //showPage(0);
    setMinimumSize(340,420);
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

ClassPropDlg::ClassPropDlg(QWidget *parent, UMLWidget *w)
        : KPageDialog(parent)
{
    init();
    m_pWidget = w;
    m_Type = pt_Widget;
    m_pObject = w -> getUMLObject();

    if (w->getBaseType() == Uml::wt_Class
            || w->getBaseType() == Uml::wt_Interface
            || w->getBaseType() == Uml::wt_Package) {
        setupPages(true);
    } else if (w->getBaseType() == Uml::wt_Component) {
        if ( w->getIsInstance() ) {
            setupInstancePages();
        } else {
            setupPages();
        }
    } else if (w->getBaseType() == Uml::wt_Node) {
        setupInstancePages();
    } else {
        setupPages();
    }

    //now setup the options page for classes
    if (w->getBaseType() == Uml::wt_Class || w->getBaseType() == Uml::wt_Interface) {
        setupDisplayPage();
    }
    setupColorPage();
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
    m_pColorPage = 0;
    m_pDoc = UMLApp::app()->getDocument();
}

ClassPropDlg::~ClassPropDlg() {}

void ClassPropDlg::slotOk() {
    slotApply();
    KDialog::accept();
}

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
    if (m_pColorPage) {
        m_pColorPage->updateUMLWidget();
    }
    if (m_pWidget) {
        m_pWidget->setFont( m_pChooser->font() );
    }
}

void ClassPropDlg::setupPages(bool assoc)
{
    setupGeneralPage();

    Uml::Object_Type ot = m_pObject->getBaseType();
    //add extra pages for class
    if (ot == Uml::ot_Class ) {
        setupAttributesPage();
    }
    if (ot == Uml::ot_Class || ot == Uml::ot_Interface) {
        setupOperationsPage();
    }
    if (ot == Uml::ot_Class || ot == Uml::ot_Interface) {
        setupTemplatesPage();
    }
    if (ot == Uml::ot_Enum) {
        setupEnumLiteralsPage();
    }
    if (ot == Uml::ot_Entity) {
        setupEntityAttributesPage();
        setupEntityConstraintsPage();
    }
    if (ot == Uml::ot_Package ) {
        setupContentsPage();
    }
    if (assoc) {
        setupAssociationsPage();
    } else {
        m_pAssocPage = 0;
    }
}

QFrame* ClassPropDlg::createPage(const QString name, const QString header, const QString icon)
{
    QFrame* page = new QFrame();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, name );
    pageItem->setHeader( header );
    pageItem->setIcon( KIcon( DesktopIcon( icon ) ));
    addPage( pageItem );
    return page;
}

void ClassPropDlg::setupGeneralPage()
{
    QFrame* page = createPage( i18n("General"), i18n("General Settings"), "preferences-other" );
    page -> setMinimumSize(310, 330);
    QHBoxLayout * topLayout = new QHBoxLayout(page);
    m_pGenPage = new ClassGenPage(m_pDoc, page, m_pObject);
    topLayout -> addWidget(m_pGenPage);
}

void ClassPropDlg::setupColorPage()
{
    QFrame * page = createPage( i18n("Color"), i18n("Widget Colors"), "preferences-desktop-color" );
    QHBoxLayout * m_pColorLayout = new QHBoxLayout(page);
    m_pColorPage = new UMLWidgetColorPage(page, m_pWidget);
    m_pColorLayout -> addWidget(m_pColorPage);
}

void ClassPropDlg::setupDisplayPage()
{
    QFrame* page = createPage( i18n("Display"), i18n("Display Options"), "preferences-desktop-theme" );
    QHBoxLayout* m_pOptionsLayout = new QHBoxLayout(page);
    ClassifierWidget *cw = static_cast<ClassifierWidget*>(m_pWidget);
    m_pOptionsPage = new ClassOptionsPage(page, cw);
    m_pOptionsLayout -> addWidget(m_pOptionsPage);
}

void ClassPropDlg::setupAttributesPage()
{
    QFrame* page = createPage( i18n("Attributes"), i18n("General Settings"), "preferences-other" );
    m_pAttPage = new ClassifierListPage(page, (UMLClassifier *)m_pObject, m_pDoc, Uml::ot_Attribute);
    QHBoxLayout * attLayout = new QHBoxLayout(page);
    attLayout -> addWidget(m_pAttPage);
}

void ClassPropDlg::setupOperationsPage()
{
    QFrame* page = createPage( i18n("Operations"), i18n("Operation Settings"), "preferences-other" );
    m_pOpsPage = new ClassifierListPage(page, (UMLClassifier*)m_pObject, m_pDoc, Uml::ot_Operation);
    QHBoxLayout* pOpsLayout = new QHBoxLayout(page);
    pOpsLayout -> addWidget(m_pOpsPage);
}

void ClassPropDlg::setupTemplatesPage()
{
    QFrame* page = createPage( i18n("Templates"), i18n("Templates Settings"), "preferences-other" );
    m_pTemplatePage = new ClassifierListPage(page, (UMLClassifier *)m_pObject, m_pDoc, Uml::ot_Template);
    QHBoxLayout* templatesLayout = new QHBoxLayout(page);
    templatesLayout->addWidget(m_pTemplatePage);
}

void ClassPropDlg::setupEnumLiteralsPage()
{
    QFrame* page = createPage( i18n("Enum Literals"), i18n("Enum Literals Settings"), "preferences-other" );
    m_pEnumLiteralPage = new ClassifierListPage(page, (UMLClassifier*)m_pObject, m_pDoc, Uml::ot_EnumLiteral);
    QHBoxLayout* enumLiteralsLayout = new QHBoxLayout(page);
    enumLiteralsLayout->addWidget(m_pEnumLiteralPage);
}

void ClassPropDlg::setupEntityAttributesPage()
{
    QFrame* page = createPage( i18n("Entity Attributes"), i18n("Entity Attributes Settings"), "preferences-other" );
    m_pEntityAttributePage = new ClassifierListPage(page, (UMLEntity*)m_pObject, m_pDoc, Uml::ot_EntityAttribute);
    QHBoxLayout* entityAttributesLayout = new QHBoxLayout(page);
    entityAttributesLayout->addWidget(m_pEntityAttributePage);
}

void ClassPropDlg::setupEntityConstraintsPage()
{
    QFrame* page = createPage( i18n("Entity Constraints"), i18n("Entity Constraints Settings"), "preferences-other" );
    m_pEntityConstraintPage = new ConstraintListPage(page, (UMLClassifier*)m_pObject, m_pDoc, Uml::ot_EntityConstraint );
    QHBoxLayout* entityConstraintsLayout = new QHBoxLayout(page);
    entityConstraintsLayout->addWidget(m_pEntityConstraintPage);
}

void ClassPropDlg::setupContentsPage()
{
    QFrame* page = createPage( i18n("Contents"), i18n("Contents Settings"), "preferences-other" );
    m_pPkgContentsPage = new PkgContentsPage(page, (UMLPackage*)m_pObject);
    QHBoxLayout* contentsLayout = new QHBoxLayout(page);
    contentsLayout->addWidget(m_pPkgContentsPage);
}

void ClassPropDlg::setupAssociationsPage()
{
    QFrame* page = createPage( i18n("Associations"), i18n("Class Associations"), "preferences-other" );
    m_pAssocPage = new AssocPage(page, UMLApp::app()->getCurrentView(), m_pObject);
    QHBoxLayout* assocLayout = new QHBoxLayout(page);
    assocLayout -> addWidget(m_pAssocPage);
}

void ClassPropDlg::setupInstancePages()
{
    QFrame* page = createPage( i18n("General"), i18n("General Settings"), "preferences-other" );
    QHBoxLayout* genLayout = new QHBoxLayout(page);
    page->setMinimumSize(310, 330);
    m_pGenPage = new ClassGenPage(m_pDoc, page, m_pWidget);
    genLayout->addWidget(m_pGenPage);
    m_pAssocPage = 0;
}

void ClassPropDlg::setupFontPage()
{
    if( !m_pWidget )
        return;
    KVBox* page = new KVBox();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, i18n("Font") );
    pageItem->setHeader( i18n("Font Settings") );
    pageItem->setIcon( KIcon(DesktopIcon("preferences-desktop-font") ));
    addPage( pageItem );
    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser -> setFont( m_pWidget -> getFont() );
}


#include "classpropdlg.moc"

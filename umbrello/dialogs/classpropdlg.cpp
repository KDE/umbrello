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
#include <qlayout.h>
//Added by qt3to4:
#include <QFrame>
#include <QHBoxLayout>
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

ClassPropDlg::ClassPropDlg(QWidget *parent, UMLObject * c, int pageNum, bool assoc)
        : KPageDialog(parent) {
    init();
    m_pWidget = 0;
    m_Type = pt_Object;

    m_pObject = c;
    setupPages(c, assoc);
#ifdef __GNUC__
#warning "kde4: reimplement showPage"
#endif
    //showPage(pageNum);
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

ClassPropDlg::ClassPropDlg(QWidget *parent, ObjectWidget * o)
        : KPageDialog(parent) {
    init();
    m_pWidget = o;
    m_Type = pt_ObjectWidget;
    m_pObject = m_pWidget->getUMLObject();
    m_pDoc = UMLApp::app()->getDocument();
    QFrame *page = new QFrame();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, i18n("General") );
    pageItem->setHeader( i18n("General Settings") );
    pageItem->setIcon( KIcon( DesktopIcon( "misc") ));
    addPage( pageItem );
    page -> setMinimumSize(310, 330);
    QHBoxLayout * topLayout = new QHBoxLayout(page);
    m_pGenPage = new ClassGenPage(m_pDoc, page, o);
    topLayout -> addWidget(m_pGenPage);

    QFrame * newPage = new QFrame();
    pageItem = new KPageWidgetItem(newPage, i18n("Color") );
    pageItem->setHeader( i18n("Widget Colors") );
    pageItem->setIcon( KIcon(DesktopIcon( "colors") ));
    addPage( pageItem);

    QHBoxLayout * m_pColorLayout = new QHBoxLayout(newPage);
    m_pColorPage = new UMLWidgetColorPage(newPage, o);
    m_pColorLayout -> addWidget(m_pColorPage);

    setupFontPage();
#ifdef __GNUC__
#warning "kde4: reimplement showPage"
#endif
    //showPage(0);
    setMinimumSize(340,420);
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}

ClassPropDlg::ClassPropDlg(QWidget *parent, UMLWidget * w)
        : KPageDialog(parent) {
    init();
    m_pWidget = w;
    m_Type = pt_Widget;
    m_pObject = w -> getUMLObject();

    if (w->getBaseType() == Uml::wt_Class
            || w->getBaseType() == Uml::wt_Interface
            || w->getBaseType() == Uml::wt_Package) {
        setupPages(m_pObject, true);
    } else if (w->getBaseType() == Uml::wt_Component) {
        if ( w->getIsInstance() ) {
            setupInstancePages(w);
        } else {
            setupPages(m_pObject);
        }
    } else if (w->getBaseType() == Uml::wt_Node) {
        setupInstancePages(w);
    } else {
        setupPages(m_pObject);
    }

    //now setup the options page for classes
    if (w->getBaseType() == Uml::wt_Class || w->getBaseType() == Uml::wt_Interface) {
        QFrame* newPage = new QFrame();
        KPageWidgetItem *pageItem = new KPageWidgetItem( newPage, i18n("Display") );
        pageItem->setHeader( i18n("Display Options") );
        pageItem->setIcon( KIcon(DesktopIcon("document-properties") ));
        addPage( pageItem);

        QHBoxLayout* m_pOptionsLayout = new QHBoxLayout(newPage);
        ClassifierWidget *cw = static_cast<ClassifierWidget*>(w);
        m_pOptionsPage = new ClassOptionsPage( newPage, cw );
        m_pOptionsLayout -> addWidget(m_pOptionsPage);
    }
    QFrame* colorPage = new QFrame();
    KPageWidgetItem *pageItem = new KPageWidgetItem( colorPage, i18n("Color") );
    pageItem->setHeader( i18n("Widget Colors") );
    pageItem->setIcon( KIcon(DesktopIcon("colors") ));
    addPage( pageItem);

    QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
    m_pColorPage = new UMLWidgetColorPage(colorPage, w);
    m_pColorLayout -> addWidget(m_pColorPage);
    setupFontPage();
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    connect(this,SIGNAL(applyClicked()),this,SLOT(slotApply()));
}


void ClassPropDlg::init() {
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

void ClassPropDlg::slotApply() {
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

void ClassPropDlg::setupPages(UMLObject * c, bool assoc) {
    QFrame* newPage = new QFrame();
    KPageWidgetItem *pageItem = new KPageWidgetItem( newPage, i18n("General") );
    pageItem->setHeader( i18n("General Settings") );
    pageItem->setIcon( KIcon(DesktopIcon("misc") ));
    addPage( pageItem);


    QHBoxLayout * genLayout = new QHBoxLayout(newPage);
    newPage -> setMinimumSize(310, 330);
    m_pGenPage = new ClassGenPage(m_pDoc, newPage, c);
    genLayout -> addWidget(m_pGenPage);
    Uml::Object_Type ot = c->getBaseType();
    //add extra pages for class
    if (ot == Uml::ot_Class ) {
        //setup attributes page
        newPage = new QFrame();
        pageItem = new KPageWidgetItem( newPage, i18n("Attributes") );
        pageItem->setHeader( i18n("General Settings") );
        pageItem->setIcon( KIcon(DesktopIcon("misc") ));
        addPage( pageItem);

        m_pAttPage = new ClassifierListPage(newPage, (UMLClassifier *)c, m_pDoc, Uml::ot_Attribute);
        QHBoxLayout * attLayout = new QHBoxLayout(newPage);
        attLayout -> addWidget(m_pAttPage);
    }

    if (ot == Uml::ot_Class || ot == Uml::ot_Interface) {
        //setup operations page
        newPage = new QFrame();
        pageItem = new KPageWidgetItem( newPage, i18n("Operations") );
        pageItem->setHeader( i18n("Operation Settings") );
        pageItem->setIcon( KIcon(DesktopIcon("misc") ));
        addPage( pageItem);
        m_pOpsPage = new ClassifierListPage(newPage, (UMLClassifier*)c, m_pDoc, Uml::ot_Operation);
        QHBoxLayout* pOpsLayout = new QHBoxLayout(newPage);
        pOpsLayout -> addWidget(m_pOpsPage);
    }

    if (ot == Uml::ot_Class || ot == Uml::ot_Interface) {
        //setup templates page
        newPage = new QFrame();
        pageItem = new KPageWidgetItem( newPage, i18n("Templates") );
        pageItem->setHeader( i18n("Templates Settings") );
        pageItem->setIcon( KIcon(DesktopIcon("misc") ));
        addPage( pageItem);
        m_pTemplatePage = new ClassifierListPage(newPage, (UMLClassifier *)c, m_pDoc, Uml::ot_Template);
        QHBoxLayout* templatesLayout = new QHBoxLayout(newPage);
        templatesLayout->addWidget(m_pTemplatePage);
    }
    if (ot == Uml::ot_Enum) {
        //setup enum literals page
        newPage = new QFrame();
        pageItem = new KPageWidgetItem( newPage, i18n("Enum Literals") );
        pageItem->setHeader( i18n("Enum Literals Settings") );
        pageItem->setIcon( KIcon(DesktopIcon("misc") ));
        addPage( pageItem);
        m_pEnumLiteralPage = new ClassifierListPage(newPage, (UMLClassifier*)c, m_pDoc, Uml::ot_EnumLiteral);
        QHBoxLayout* enumLiteralsLayout = new QHBoxLayout(newPage);
        enumLiteralsLayout->addWidget(m_pEnumLiteralPage);
    }
    if (ot == Uml::ot_Entity) {
        //setup entity attributes page
        newPage = new QFrame();
        pageItem = new KPageWidgetItem( newPage, i18n("Entity Attributes") );
        pageItem->setHeader( i18n("Entity Attributes Settings") );
        pageItem->setIcon( KIcon(DesktopIcon("misc") ));
        addPage( pageItem);
        m_pEntityAttributePage = new ClassifierListPage(newPage, (UMLEntity*)c, m_pDoc, Uml::ot_EntityAttribute);
        QHBoxLayout* entityAttributesLayout = new QHBoxLayout(newPage);
        entityAttributesLayout->addWidget(m_pEntityAttributePage);

        // setup constraints page
        newPage = new QFrame();
        pageItem = new KPageWidgetItem( newPage, i18n( "Entity Constraints" ) );
        pageItem->setHeader( i18n( "Entity Constraints Settings" ) );
        pageItem->setIcon( KIcon( DesktopIcon( "misc" ) ) );
        addPage( pageItem );
        m_pEntityConstraintPage = new ConstraintListPage( newPage, ( UMLClassifier* )c, m_pDoc, Uml::ot_EntityConstraint );
        QHBoxLayout* entityConstraintsLayout = new QHBoxLayout( newPage );
        entityConstraintsLayout->addWidget( m_pEntityConstraintPage );
    }

    if (ot == Uml::ot_Package ) {
        // Set up containment page.
        newPage = new QFrame();
        pageItem = new KPageWidgetItem( newPage, i18n("Contents") );
        pageItem->setHeader( i18n("Contents Settings") );
        pageItem->setIcon( KIcon(DesktopIcon("misc") ));
        addPage( pageItem);
        m_pPkgContentsPage = new PkgContentsPage(newPage, (UMLPackage*)(c));
        QHBoxLayout* contentsLayout = new QHBoxLayout(newPage);
        contentsLayout->addWidget(m_pPkgContentsPage);
    }
    if (assoc) {
        newPage = new QFrame();
        pageItem = new KPageWidgetItem( newPage, i18n("Associations") );
        pageItem->setHeader( i18n("Class Associations") );
        pageItem->setIcon( KIcon(DesktopIcon("misc") ));
        addPage( pageItem);
        m_pAssocPage = new AssocPage(newPage, UMLApp::app()->getCurrentView(), m_pObject);
        QHBoxLayout* assocLayout = new QHBoxLayout(newPage);
        assocLayout -> addWidget(m_pAssocPage);
    } else {
        m_pAssocPage = 0;
    }
}

void ClassPropDlg::setupInstancePages(UMLWidget* widget) {
    QFrame* page = new QFrame();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, i18n("General") );
    pageItem->setHeader( i18n("General Settings") );
    pageItem->setIcon( KIcon(DesktopIcon("misc") ));
    addPage( pageItem);

    QHBoxLayout* genLayout = new QHBoxLayout(page);
    page->setMinimumSize(310, 330);
    m_pGenPage = new ClassGenPage(m_pDoc, page, widget);
    genLayout->addWidget(m_pGenPage);
    m_pAssocPage = 0;
}

void ClassPropDlg::setupFontPage() {
    if( !m_pWidget )
        return;
    KVBox* page = new KVBox();
    KPageWidgetItem *pageItem = new KPageWidgetItem( page, i18n("Font") );
    pageItem->setHeader( i18n("Font Settings") );
    pageItem->setIcon( KIcon(DesktopIcon("fonts") ));
    addPage( pageItem);
    m_pChooser = new KFontChooser( (QWidget*)page, false, QStringList(), false);
    m_pChooser -> setFont( m_pWidget -> getFont() );
}


#include "classpropdlg.moc"

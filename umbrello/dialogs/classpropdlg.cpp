/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#include "classpropdlg.h"
#include "classgenpage.h"
#include "classifierlistpage.h"
#include "assocpage.h"
#include "classoptionspage.h"
#include "umlwidgetcolorpage.h"
#include "../umlobject.h"
#include "../umldoc.h"
#include "../objectwidget.h"
#include "../componentwidget.h"
#include "../uml.h"
#include "../umlview.h"

ClassPropDlg::ClassPropDlg(QWidget *parent, UMLObject * c, int pageNum, bool assoc) : KDialogBase(IconList, i18n("Properties"), Ok | Apply | Cancel | Help,
        Ok, parent, "_CLASSDLG_", true, true) {
	m_pWidget = 0;
	m_pGenPage = 0;
	m_pAttPage = 0;
	m_pOpsPage = 0;
	m_pTemplatePage = 0;
	m_pOptionsPage = 0;
	m_pColorPage = 0;
	m_Type = pt_Object;
	m_pDoc = ((UMLApp *)parent) -> getDocument();
	m_pObject = c;
	setupPages(c, assoc);
	showPage(pageNum);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassPropDlg::ClassPropDlg(QWidget *parent, ObjectWidget * o) : KDialogBase(IconList, i18n("Properties"), Ok | Apply | Cancel | Help,
        Ok, parent, "_CLASSDLG_", true, true) {
	m_pWidget = o;
	m_pGenPage = 0;
	m_pAttPage = 0;
	m_pOpsPage = 0;
	m_pTemplatePage = 0;
	m_pOptionsPage = 0;
	m_Type = pt_ObjectWidget;
	m_pObject = m_pWidget->getUMLObject();
	m_pDoc = ((UMLApp *)parent) -> getDocument();
	QFrame *page = addPage( i18n("General"), i18n("General Settings"), DesktopIcon( "misc") );
	page -> setMinimumSize(310, 330);
	QHBoxLayout * topLayout = new QHBoxLayout(page);
	m_pGenPage = new ClassGenPage(m_pDoc, page, o);
	topLayout -> addWidget(m_pGenPage);

	QFrame * newPage = addPage( i18n("Color"), i18n("Widget Colors"), DesktopIcon( "colors") );
	QHBoxLayout * m_pColorLayout = new QHBoxLayout(newPage);
	m_pColorPage = new UMLWidgetColorPage(newPage, o);
	m_pColorLayout -> addWidget(m_pColorPage);

	setupFontPage();
	showPage(0);
	setMinimumSize(340,420);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassPropDlg::ClassPropDlg(QWidget *parent, UMLWidget * w) : KDialogBase(IconList, i18n("Properties"), Ok | Apply | Cancel | Help,
        Ok, parent, "_CLASSDLG_", true, true) {
	m_pWidget = w;
	m_pGenPage = 0;
	m_pAttPage = 0;
	m_pOpsPage = 0;
	m_pTemplatePage = 0;
	m_pOptionsPage = 0;
	m_Type = pt_Widget;
	m_pObject = w -> getUMLObject();
	m_pDoc = ((UMLApp *)parent) -> getDocument();

	if (w->getBaseType() == Uml::wt_Class
		|| w->getBaseType() == Uml::wt_Interface
		|| w->getBaseType() == Uml::wt_Package
	) {
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
		QFrame* newPage = addPage( i18n("Display"), i18n("Display Options"), DesktopIcon("info") );
		QHBoxLayout* m_pOptionsLayout = new QHBoxLayout(newPage);
		m_pOptionsPage = new ClassOptionsPage( newPage, w, w->getBaseType() );
		m_pOptionsLayout -> addWidget(m_pOptionsPage);
	}

	QFrame* colorPage = addPage( i18n("Color"), i18n("Widget Colors"), DesktopIcon("colors") );
	QHBoxLayout * m_pColorLayout = new QHBoxLayout(colorPage);
	m_pColorPage = new UMLWidgetColorPage(colorPage, w);
	m_pColorLayout -> addWidget(m_pColorPage);
	setupFontPage();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassPropDlg::~ClassPropDlg() {}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassPropDlg::slotOk() {
	slotApply();
	KDialogBase::accept();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassPropDlg::slotUpdateChildObject(int id) {

	UMLObject * o; // = ((UMLClassifier*)m_pObject) -> findChildObject(id);

	if (typeid(m_pObject) == typeid(UMLClass))
		o = ((UMLClass *)m_pObject) -> findChildObject(id);
	else
		o = ((UMLClassifier *)m_pObject) -> findChildObject(id);

}
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassPropDlg::setupPages(UMLObject * c, bool assoc) {
	QFrame *page = addPage(i18n("General"), i18n("General Settings"), DesktopIcon( "misc") );
	QHBoxLayout * genLayout = new QHBoxLayout(page);
	page -> setMinimumSize(310, 330);
	m_pGenPage = new ClassGenPage(m_pDoc, page, c);
	genLayout -> addWidget(m_pGenPage);
	//add extra pages for class
	if (c->getBaseType() == Uml::ot_Class ) {
		//setup attributes page
		QFrame* newPage = addPage( i18n("Attributes"), i18n("Attribute Settings"), DesktopIcon("misc") );
		m_pAttPage = new ClassifierListPage(newPage, (UMLClassifier *)c, m_pDoc, ot_Attribute);
		QHBoxLayout * attLayout = new QHBoxLayout(newPage);
		attLayout -> addWidget(m_pAttPage);
		connect(m_pAttPage, SIGNAL(sigUpdateChildObject(int)), this, SLOT(slotUpdateChildObject(int)));
	}

	if (c->getBaseType() == Uml::ot_Class || c->getBaseType() == Uml::ot_Interface) {

		//setup operations page
		QFrame* newPage = addPage( i18n("Operations"), i18n("Operation Settings"), DesktopIcon("misc") );
		m_pOpsPage = new ClassifierListPage(newPage, (UMLClassifier*)c, m_pDoc, ot_Operation);
		QHBoxLayout* pOpsLayout = new QHBoxLayout(newPage);
		pOpsLayout -> addWidget(m_pOpsPage);
		connect(m_pOpsPage, SIGNAL(sigUpdateChildObject(int)), this, SLOT(slotUpdateChildObject(int)));
	}

	if (c->getBaseType() == Uml::ot_Class ) {
		//setup templates page
		QFrame* newPage = addPage( i18n("Templates"), i18n("Templates Settings"), DesktopIcon("misc") );
		m_pTemplatePage = new ClassifierListPage(newPage, (UMLClassifier *)c, m_pDoc, ot_Template);
		QHBoxLayout* templatesLayout = new QHBoxLayout(newPage);
		templatesLayout->addWidget(m_pTemplatePage);
		connect(m_pTemplatePage, SIGNAL(sigUpdateChildObject(int)), this, SLOT(slotUpdateChildObject(int)));
	}
	if (assoc) {
		QFrame* newPage = addPage(i18n("Associations"), i18n("Class Associations"), DesktopIcon( "misc") );
		m_pAssocPage = new AssocPage(newPage, m_pDoc -> getCurrentView(), m_pObject);
		QHBoxLayout* assocLayout = new QHBoxLayout(newPage);
		assocLayout -> addWidget(m_pAssocPage);
	} else {
		m_pAssocPage = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassPropDlg::setupInstancePages(UMLWidget* widget) {
	QFrame* page = addPage( i18n("General"), i18n("General Settings"), DesktopIcon("misc") );
	QHBoxLayout* genLayout = new QHBoxLayout(page);
	page->setMinimumSize(310, 330);
	m_pGenPage = new ClassGenPage(m_pDoc, page, widget);
	genLayout->addWidget(m_pGenPage);
	m_pAssocPage = 0;
}

void ClassPropDlg::setupFontPage() {
	if( !m_pWidget )
		return;
	QVBox * page = addVBoxPage( i18n("Font"), i18n("Font Settings"), DesktopIcon( "fonts")  );
	m_pChooser = new KFontChooser( (QWidget*)page, "font", false, QStringList(), false);
	m_pChooser -> setFont( m_pWidget -> getFont() );
}



#include "classpropdlg.moc"

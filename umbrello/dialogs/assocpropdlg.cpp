/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qlayout.h>
#include <qlabel.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "assocpropdlg.h"
#include "assocgenpage.h"
#include "assocrolepage.h"

#include "classpropdlg.h"
#include "classgenpage.h"
#include "umlwidgetcolorpage.h"

// #include "../umlobject.h"
// #include "../umldoc.h"
// #include "../objectwidget.h"
// #include "../uml.h"
// #include "../umlview.h"
#include "../association.h"

AssocPropDlg::AssocPropDlg (QWidget *parent, UMLAssociation *assoc, int pageNum)
 	: KDialogBase(IconList, i18n("Association Properties"), Ok | Apply | Cancel | Help,
		      Ok, parent, "_ASSOCPROPDLG_", true, true)
{
 	m_assoc = assoc;
	m_pGenPage  = 0;
 	m_pRolePage = 0;

 	setupPages( );
 	showPage(pageNum);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
AssocPropDlg::~AssocPropDlg() { }


////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocPropDlg::slotOk() {
 	slotApply();
 	KDialogBase::accept();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocPropDlg::slotApply() {

 	if (m_pGenPage) {
 		m_pGenPage->updateObject();
  	}

 	if (m_pRolePage) {
 		m_pRolePage->updateObject();
  	}

//  	if (m_pAssoc) {
//  		m_pAssoc->setFont( m_pChooser->font() );
//   	}


}

////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocPropDlg::setupPages ( )
{
 	// general page
 	QFrame *page = addPage( i18n("General"), i18n("General Settings"), DesktopIcon( "misc") );
 	QHBoxLayout *genLayout = new QHBoxLayout(page);
 	page -> setMinimumSize(310, 330);
 	m_pGenPage = new AssocGenPage ( page, m_assoc);
 	genLayout -> addWidget(m_pGenPage);

 	// role page
 	QFrame * newPage = addPage( i18n("Roles"), i18n("Role Settings"), DesktopIcon( "misc") );
 	QHBoxLayout * roleLayout = new QHBoxLayout(newPage);
 	// newPage -> setMinimumSize(310, 330);
 	m_pRolePage = new AssocRolePage( newPage, m_assoc );
 	roleLayout -> addWidget(m_pRolePage);

 	setupFontPage();
}

void AssocPropDlg::setupFontPage()
{
//  	if( !m_assoc )
//  		return;
// 
//  	QVBox *page = addVBoxPage( i18n("Font"), i18n("Font Settings"), DesktopIcon( "fonts"));
//  	m_pChooser = new KFontChooser( (QWidget*)page, "font", false, QStringList(), false);
//  	m_pChooser->setFont( m_pAssoc->getFont());
//  	m_pChooser->setSampleText(i18n("Association font"));
}

/////
QString AssocPropDlg::getName() const { 
	return m_assoc->getName(); 
} 

QString AssocPropDlg::getRoleAName() const { 
	return m_assoc->getRoleNameA(); 
}

QString AssocPropDlg::getDoc() const { 
	return m_assoc->getDoc(); 
}

QString AssocPropDlg::getRoleADoc() const { 
	return m_assoc->getRoleADoc(); 
}

QString AssocPropDlg::getRoleBName() const { 
	return m_assoc->getRoleNameB(); 
} 

QString AssocPropDlg::getRoleBDoc() const { 
	return m_assoc->getRoleBDoc(); 
}

QString AssocPropDlg::getMultiA() const { 
	return m_assoc->getMultiA(); 
} 

QString AssocPropDlg::getMultiB() const { 
	return m_assoc->getMultiB(); 
} 

Uml::Scope AssocPropDlg::getVisibilityA() const { 
	return m_assoc->getVisibilityA(); 
}

Uml::Scope AssocPropDlg::getVisibilityB() const { 
	return m_assoc->getVisibilityB(); 
}

Uml::Changeability_Type AssocPropDlg::getChangeabilityA() const { 
	return m_assoc->getChangeabilityA(); 
}

Uml::Changeability_Type AssocPropDlg::getChangeabilityB() const { 
	return m_assoc->getChangeabilityB(); 
}



#include "assocpropdlg.moc"

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classwizard.h"
#include "../attribute.h"
#include "../operation.h"
#include "../umllistview.h"
#include <khelpmenu.h>
#include <klocale.h>

ClassWizard::ClassWizard( UMLDoc * pDoc ) : KWizard( (QWidget*)pDoc -> parent(), "_CLASSWIZARD_", true) {
	m_pDoc = pDoc;
	m_pDoc -> getListView() -> setLoading( true );
	//create a unique class to start with
	UMLObject * pTemp = 0;
	QString name = i18n("new_class");
	QString newName = name;
	QString num = "";
	int i = 0;
	m_pClass = new UMLClass( m_pDoc, newName, m_pDoc -> getUniqueID() );
	do {
		m_pClass -> setName( newName );
		pTemp = m_pDoc -> findUMLObject( Uml::ot_Class, newName );
		num.setNum( ++i);
		newName = name;
		newName.append("_").append( num );
	} while( pTemp );
	//setup pages
	setupPages();
}

ClassWizard::~ClassWizard() {}

void ClassWizard::setupPages() {
	//Setup General Page
	m_pGenPage = new ClassGenPage( m_pDoc, this, m_pClass );
	addPage( m_pGenPage, i18n("New Class") );
	setHelpEnabled(m_pGenPage, false);

	//Setup Attribute Page
	m_pAttPage = new ClassAttPage( this, m_pClass, m_pDoc );
	addPage( m_pAttPage, i18n("Class Attributes") );

	//Setup Operation Page
	m_pOpPage = new ClassOpsPage( this, m_pClass, m_pDoc );
	addPage( m_pOpPage, i18n("Class Operations") );
}

void ClassWizard::showPage( QWidget * pWidget ) {
	QWizard::showPage( pWidget );
	if( pWidget == m_pOpPage )
		finishButton() -> setEnabled( true );
}

void ClassWizard::next() {
	QWidget * pWidget = currentPage();
	if( pWidget == m_pGenPage ) {
		m_pGenPage -> updateObject();
	} else if( pWidget == m_pAttPage ) {
		m_pAttPage -> updateObject();
	}
	QWizard::next();
}

void ClassWizard::back() {
	QWidget * pWidget = currentPage();
	if( pWidget == m_pAttPage ) {
		m_pAttPage -> updateObject();
	} else if( pWidget == m_pOpPage ) {
		m_pOpPage -> updateObject();
	}
	QWizard::back();
}

void ClassWizard::accept() {
	m_pDoc -> addUMLObject( m_pClass );
	//add signal code here for list view
	m_pDoc -> getListView() -> setLoading( false );
	m_pDoc -> signalUMLObjectCreated( m_pClass );
	UMLObject * pObject = 0;
	QPtrList <UMLAttribute> * attList = m_pClass -> getAttList();
	for( pObject = attList -> first(); pObject; pObject = attList -> next() )
		m_pDoc -> signalChildUMLObjectCreated( pObject );
	QPtrList <UMLOperation> * opList = m_pClass -> getOpList();
	for( pObject = opList -> first(); pObject; pObject = opList -> next() )
		m_pDoc -> signalChildUMLObjectCreated( pObject );
	QWizard::accept();
}

void ClassWizard::reject() {
	delete m_pClass;
	m_pDoc -> getListView() -> setLoading( false );
	QWizard::reject();
}

void ClassWizard::help() {
	KHelpMenu helpMenu(this);
	helpMenu.appHelpActivated();
}

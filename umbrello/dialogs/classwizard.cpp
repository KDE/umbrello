/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "classwizard.h"

// system includes
#include <khelpmenu.h>
#include <klocale.h>

// local includes
#include "classifierlistpage.h"
#include "../uml.h"
#include "../attribute.h"
#include "../operation.h"
#include "../umllistview.h"
#include "../umlclassifierlistitemlist.h"
#include "../classifierlistitem.h"

ClassWizard::ClassWizard( UMLDoc * pDoc ) : KWizard( (QWidget*)pDoc -> parent(), "_CLASSWIZARD_", true) {
	m_pDoc = pDoc;
	//create a unique class to start with
	UMLObject * pTemp = 0;
	QString name = i18n("new_class");
	QString newName = name;
	QString num = "";
	int i = 0;
	m_pClass = new UMLClass( newName, m_pDoc -> getUniqueID() );
	do {
		m_pClass -> setName( newName );
		pTemp = m_pDoc -> findUMLObject( newName );
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
	m_pAttPage = new ClassifierListPage(this, m_pClass, m_pDoc, ot_Attribute);
	addPage( m_pAttPage, i18n("Class Attributes") );

	//Setup Operation Page
	m_pOpPage = new ClassifierListPage(this, m_pClass, m_pDoc, ot_Operation);
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
	m_pDoc->signalUMLObjectCreated(m_pClass);

	UMLListView *listView = UMLApp::app()->getListView();
	UMLClassifierListItemList attributes = m_pClass->getFilteredList(ot_Attribute);
	for ( UMLClassifierListItem* attribute = attributes.first(); attribute; attribute = attributes.next() )  {
		listView->childObjectAdded(attribute, m_pClass);
	}

	UMLOperationList operations = m_pClass->getOpList();
	for ( UMLOperation* operation = operations.first(); operation; operation = operations.next() )  {
		listView->childObjectAdded(operation, m_pClass);
	}

	QWizard::accept();
}

void ClassWizard::reject() {
	delete m_pClass;
	QWizard::reject();
}

void ClassWizard::help() {
	KHelpMenu helpMenu(this);
	helpMenu.appHelpActivated();
}

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "parmpropdlg.h"
#include "../classifier.h"
#include "../interface.h"
#include "../umldoc.h"
#include <klocale.h>
#include <kdebug.h>
#include <qlayout.h>

ParmPropDlg::ParmPropDlg(QWidget * parent, UMLDoc * doc, UMLAttribute * a)
  : KDialogBase(Plain, i18n("Parameter Properties"), Help | Ok | Cancel , Ok, parent, "_PARMPROPDLG_", true, true)
{
	m_pUmldoc = doc;
	m_pAtt = a;
	QString type, text, name, initialValue;
	if(!a) {
		type = text = name = initialValue = "";
	} else {
		type = a -> getTypeName();
		name = a -> getName();
		initialValue = a -> getInitialValue();
		text = a -> getDoc();
	}
	int margin = fontMetrics().height();
	setMinimumSize(300, 400);
	//disableResize();
	QVBoxLayout * topLayout = new QVBoxLayout(plainPage());
	topLayout -> setSpacing(10);
	topLayout -> setMargin(margin);

	m_pKind =  new QButtonGroup(i18n("Kind"), plainPage());
	m_pKind->setExclusive(true);
	m_pKindLayout = new QHBoxLayout( m_pKind );
	m_pKindLayout->setMargin(margin);
	// m_pKind->setTitle("Kind");
	//m_pKind->setColumnLayout(0, Qt::Vertical );
	//m_pKind->layout()->setSpacing( 6 );
	//m_pKindLayout->setAlignment( Qt::AlignTop );

	m_pIn =  new QRadioButton( "in", m_pKind );
	m_pKindLayout->addWidget( m_pIn );

	m_pInOut =  new QRadioButton( "inout", m_pKind );
	m_pKindLayout->addWidget( m_pInOut );

	m_pOut =  new QRadioButton( "out", m_pKind );
	m_pKindLayout->addWidget( m_pOut );

	topLayout -> addWidget(m_pKind);

	m_pParmGB = new QGroupBox(i18n("Properties"), plainPage());
	topLayout -> addWidget(m_pParmGB);

	QGridLayout * propLayout = new QGridLayout(m_pParmGB, 3, 2);
	propLayout -> setSpacing(10);
	propLayout -> setMargin(margin);

	m_pTypeL = new QLabel(i18n("&Type:"), m_pParmGB);
	propLayout -> addWidget(m_pTypeL, 0, 0);

	m_pTypeCB = new QComboBox(m_pParmGB);
	propLayout -> addWidget(m_pTypeCB, 0, 1);
	m_pTypeL->setBuddy(m_pTypeCB);

	m_pNameL = new QLabel(i18n("&Name:"), m_pParmGB);
	propLayout -> addWidget(m_pNameL, 1, 0);

	m_pNameLE = new QLineEdit(m_pParmGB);
	m_pNameLE -> setText(name);
	propLayout -> addWidget(m_pNameLE, 1, 1);
	m_pNameL->setBuddy(m_pNameLE);

	m_pInitialL = new QLabel(i18n("&Initial value:"), m_pParmGB);
	propLayout -> addWidget(m_pInitialL, 2, 0);

	m_pInitialLE = new QLineEdit(m_pParmGB);
	m_pInitialLE -> setText(initialValue);
	propLayout -> addWidget(m_pInitialLE, 2, 1);
	m_pInitialL->setBuddy(m_pInitialLE);

	m_pDocGB = new QGroupBox(i18n("Documentation"), plainPage());
	QHBoxLayout * docLayout = new QHBoxLayout(m_pDocGB);
	docLayout -> setMargin(margin);

	m_pDoc = new QMultiLineEdit(m_pDocGB);
	///////////
	m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
	//////////
	m_pDoc -> setText(text);
	docLayout -> addWidget(m_pDoc);
	topLayout -> addWidget(m_pDocGB);

	// Check the proper Kind radiobutton.
	if (a) {
		Uml::Parameter_Kind kind = a->getParmKind();
		if (kind == Uml::pk_Out)
			m_pOut->setChecked(true);
		else if (kind == Uml::pk_InOut)
			m_pInOut->setChecked(true);
		else
			m_pIn->setChecked(true);
	} else
		m_pIn->setChecked(true);

	//add some standard attribute types to combo box
	QString types[] ={i18n("int"), i18n("long"), i18n("bool"), i18n("string"), i18n("double"), i18n("float"), i18n("date")};
	for (int i=0; i<7; i++) {
		m_pTypeCB->insertItem(types[i]);
	}

	m_pTypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
	m_pTypeCB->setEditable(true);
	m_pTypeCB->setAutoCompletion(true);

	//now add the Concepts
	UMLClassifierList namesList( m_pUmldoc->getConcepts() );
	UMLClassifier * obj;
	for(obj=namesList.first(); obj!=0 ;obj=namesList.next()) {
		m_pTypeCB->insertItem( obj->getName() );
	}
	UMLInterfaceList interfaceList( m_pUmldoc->getInterfaces() );
	UMLInterface* pInterface = 0;
	for(pInterface=interfaceList.first(); pInterface!=0 ;pInterface=interfaceList.next()) {
		m_pTypeCB->insertItem( pInterface->getName() );
	}

	//work out which one to select
	int typeBoxCount = 0;
	bool foundType = false;
	while (typeBoxCount < m_pTypeCB->count() && foundType == false) {
		QString typeBoxString = m_pTypeCB->text(typeBoxCount);
		if ( typeBoxString == type ) { //getTypeName()
			foundType = true;
			m_pTypeCB->setCurrentItem(typeBoxCount);
		} else {
			typeBoxCount++;
		}
	}

	if (!foundType) {
		m_pTypeCB->insertItem( type, 0 );
		m_pTypeCB->setCurrentItem(0);
	}

	m_pNameLE->setFocus();
}

void ParmPropDlg::slotOk() {
	if (m_pAtt != NULL) {
		if (m_pOut->isChecked())
			m_pAtt->setParmKind(Uml::pk_Out);
		else if (m_pInOut->isChecked())
			m_pAtt->setParmKind(Uml::pk_InOut);
		else
			m_pAtt->setParmKind(Uml::pk_In);
	}
	accept();
}

ParmPropDlg::~ParmPropDlg() {}


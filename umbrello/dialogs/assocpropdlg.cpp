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

#include "assocpropdlg.h"
#include "../associationwidgetdata.h"
#include "../umlwidget.h"
#include "../umlobject.h"
#include "../floatingtext.h"
#include "../assocrules.h"

AssocPropDlg::AssocPropDlg(QWidget * parent, AssociationWidget * a) : KDialogBase(Plain, i18n("Association Properties"), Help | Ok | Cancel , Ok, parent, "_ASSOCPROPDLG_", true, true) {
	assocWidget = a;
	int margin = fontMetrics().height();
	QString types[] = {i18n("Generalization"), i18n("Dependancy"), i18n("Aggregation"), i18n("Association"),
	                   i18n("Implementation"), i18n("Composition"), i18n("Unidirectional Association"), i18n("Realization"), i18n("State Transition") };

	m_pAssocGB = new QGroupBox(i18n("Properties"), plainPage());
	QVBoxLayout * mainLayout = new QVBoxLayout(plainPage());
	mainLayout -> addWidget(m_pAssocGB);
	mainLayout -> setSpacing(10);
	mainLayout -> setMargin(margin);

	QGridLayout * layout = new QGridLayout(m_pAssocGB, 3, 4);
	layout -> setSpacing(10);
	layout -> setMargin(margin);
	QString nameA = i18n("Name"), nameB = i18n("Name");

	layout -> addWidget(new QLabel(nameA, m_pAssocGB), 0, 0);
	m_pNameALE = new QLineEdit(m_pAssocGB);
	layout -> addWidget(m_pNameALE, 0, 1);

	layout -> addWidget(new QLabel(i18n("Multiplicity"), m_pAssocGB), 0, 2);
	m_pMultiALE = new QLineEdit(m_pAssocGB);
	layout -> addWidget(m_pMultiALE, 0, 3);

	layout -> addWidget(new QLabel(i18n("Type"), m_pAssocGB), 1, 0);
	m_pTypeLE = new QLineEdit(m_pAssocGB);
	layout -> addWidget(m_pTypeLE, 1, 1);

	layout -> addWidget(new QLabel(i18n("Role name"), m_pAssocGB), 1, 2);
	m_pRoleNameLE = new QLineEdit(m_pAssocGB);
	layout -> addWidget(m_pRoleNameLE, 1, 3);

	layout -> addWidget(new QLabel(nameB, m_pAssocGB), 2, 0);
	m_pNameBLE = new QLineEdit(m_pAssocGB);
	layout -> addWidget(m_pNameBLE, 2, 1);

	layout -> addWidget(new QLabel(i18n("Multiplicity"), m_pAssocGB), 2, 2);
	m_pMultiBLE = new QLineEdit(m_pAssocGB);
	layout -> addWidget(m_pMultiBLE, 2, 3);

	m_pDocGB = new QGroupBox(i18n("Documentation"), plainPage());
	mainLayout -> addWidget( m_pDocGB );
	QHBoxLayout * docLayout = new QHBoxLayout(m_pDocGB);
	docLayout -> setSpacing(10);
	docLayout -> setMargin(margin);

	m_pDocMLE = new QMultiLineEdit( m_pDocGB );
	docLayout -> addWidget( m_pDocMLE );
	m_pDocMLE -> setWordWrap( QMultiLineEdit::WidgetWidth) ;

	//fill widgets
	m_pDocMLE -> setText( a -> getDoc() );
	UMLWidget * wA = a -> getWidgetA();
	UMLWidget * wB = a -> getWidgetB();
	Uml::Association_Type assoc = a -> getAssocType();

	bool allowMulti = AssocRules::allowMultiplicity( assoc, wA -> getBaseType() );
	bool allowRole = AssocRules::allowRole( assoc );
	switch(assoc) {
		case Uml::at_Generalization:
			m_pTypeLE -> setText(types[0]);
			break;

		case Uml::at_Association:
			m_pTypeLE -> setText(types[3]);
			break;

		case Uml::at_Aggregation:
			m_pTypeLE -> setText(types[2]);
			break;

		case Uml::at_Dependency:
			m_pTypeLE -> setText(types[1]);
			break;

		case Uml::at_Implementation:
			m_pTypeLE -> setText(types[4]);
			break;

		case Uml::at_Composition:
			m_pTypeLE -> setText(types[5]);
			break;

		case Uml::at_UniAssociation:
			m_pTypeLE -> setText(types[6]);
			break;

		case Uml::at_Realization:
			m_pTypeLE -> setText(types[7]);
			break;

		case Uml::at_State:
			m_pTypeLE -> setText(types[8]);
			break;

		default:
			break;
	}//end switch
	//don't start with invalid text in the edited boxes
	QString rt = a -> getRole();
	QString mat  = a -> getMultiA();
	QString mbt = a -> getMultiB();
	if(!FloatingText::isTextValid(rt))
		rt = "";
	if(!FloatingText::isTextValid(mat))
		mat = "";
	if(!FloatingText::isTextValid(mbt))
		mbt = "";
	m_pRoleNameLE -> setText(rt);
	m_pMultiALE -> setText(mat);
	m_pMultiBLE -> setText(mbt);

	m_pNameALE -> setText(wA -> getName());
	m_pNameBLE -> setText(wB -> getName());
	m_pNameALE -> setEnabled(false);
	m_pNameBLE -> setEnabled(false);
	m_pTypeLE -> setEnabled(false);
	m_pRoleNameLE -> setEnabled( allowRole );
	m_pMultiALE -> setEnabled( allowMulti );
	m_pMultiBLE -> setEnabled( allowMulti );
	setMinimumSize(330, 160);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
AssocPropDlg::~AssocPropDlg() {}

void AssocPropDlg::slotOk() {
	assocWidget -> setDoc( m_pDocMLE -> text() );
	QString a = m_pMultiALE -> text();
	QString b = m_pMultiBLE -> text();
	QString r = m_pRoleNameLE -> text();
	//no text is valid but if something has been entered, then check it.

	if(a.length() > 0 && !FloatingText::isTextValid(a)) {
		KMessageBox::error(this, i18n("An invalid entry has been entered.\n Please correct this problem."));
		return;
	}
	if(b.length() > 0 && !FloatingText::isTextValid(b)) {
		KMessageBox::error(this, i18n("An invalid entry has been entered.\n Please correct this problem."));
		return;
	}
	if(r.length() > 0 && !FloatingText::isTextValid(r)) {
		KMessageBox::error(this, i18n("An invalid entry has been entered.\n Please correct this problem."));
		return;
	}
	KDialogBase::accept();
}


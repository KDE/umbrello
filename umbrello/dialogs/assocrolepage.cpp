/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "assocrolepage.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "../association.h"



AssocRolePage::AssocRolePage( QWidget *parent, UMLAssociation *assoc)
	: QWidget(parent)
{

	m_assoc = assoc;
	
	m_pRoleALE = 0;
	m_pRoleBLE = 0;
	m_pMultiALE = 0;
	m_pMultiBLE = 0;

	constructWidget();

}

AssocRolePage::~AssocRolePage() {}

void AssocRolePage::constructWidget() {

	// general configuration of the GUI
	int margin = fontMetrics().height();
	setMinimumSize(310,330);
	QGridLayout * mainLayout = new QGridLayout(this, 4, 2);
	mainLayout -> setSpacing(6);

	// group boxes for role, documentation properties
	QGroupBox *propsAGB = new QGroupBox(this);
	QGroupBox *propsBGB = new QGroupBox(this);
        QButtonGroup * scopeABG = new QButtonGroup(i18n("Role A Visibility"), this );
        QButtonGroup * scopeBBG = new QButtonGroup(i18n("Role B Visibility"), this );
        QButtonGroup * changeABG = new QButtonGroup(i18n("Role A Changeability"), this );
        QButtonGroup * changeBBG = new QButtonGroup(i18n("Role B Changeability"), this );
	QGroupBox *docAGB = new QGroupBox(this);
	QGroupBox *docBGB = new QGroupBox(this);
	propsAGB -> setTitle(i18n("Role A Properties"));
	propsBGB -> setTitle(i18n("Role B Properties"));
	docAGB -> setTitle(i18n("Documentation"));
	docBGB -> setTitle(i18n("Documentation"));
	mainLayout -> addWidget( propsAGB, 0, 0);
	mainLayout -> addWidget( scopeABG, 1, 0);
	mainLayout -> addWidget(changeABG, 2, 0);
	mainLayout -> addWidget(   docAGB, 3, 0);
	mainLayout -> addWidget( propsBGB, 0, 1);
	mainLayout -> addWidget( scopeBBG, 1, 1);
	mainLayout -> addWidget(changeBBG, 2, 1);
	mainLayout -> addWidget(   docBGB, 3, 1);

	QGridLayout * propsALayout = new QGridLayout(propsAGB, 2, 2);
	propsALayout -> setSpacing(6);
	propsALayout -> setMargin(margin);

	QGridLayout * propsBLayout = new QGridLayout(propsBGB, 3, 2);
	propsBLayout -> setSpacing(6);
	propsBLayout -> setMargin(margin);

	// Properties
	//

	// Rolename A
	propsALayout -> addWidget(new QLabel(i18n("Rolename:"),propsAGB), 0, 0);
	m_pRoleALE = new QLineEdit(propsAGB);
	propsALayout -> addWidget(m_pRoleALE, 0, 1);
	m_pRoleALE -> setText(m_assoc->getRoleNameA());

	// Multi A
	propsALayout -> addWidget(new QLabel(i18n("Multiplicity:"),propsAGB), 1, 0);
	m_pMultiALE = new QLineEdit(propsAGB);
	propsALayout -> addWidget(m_pMultiALE, 1, 1);
	m_pMultiALE -> setText(m_assoc->getMultiA());

	// Visibility A
        QHBoxLayout * scopeALayout = new QHBoxLayout(scopeABG);
        scopeALayout -> setMargin(margin);

        m_PublicARB = new QRadioButton(i18n("Public"), scopeABG);
        scopeALayout -> addWidget(m_PublicARB);

        m_PrivateARB = new QRadioButton(i18n("Private"), scopeABG);
        scopeALayout -> addWidget(m_PrivateARB);

        m_ProtectedARB = new QRadioButton(i18n("Protected"), scopeABG);
        scopeALayout -> addWidget(m_ProtectedARB);

        Uml::Scope scope = m_assoc->getVisibilityA();
        if( scope == Uml::Public )
                m_PublicARB -> setChecked( true );
        else if( scope == Uml::Private )
                m_PrivateARB -> setChecked( true );
        else
                m_ProtectedARB -> setChecked( true );

	// Changeability A
	QHBoxLayout * changeALayout = new QHBoxLayout(changeABG);
	changeALayout -> setMargin(margin);

	m_ChangeableARB = new QRadioButton(i18n("Changeable"), changeABG);
	changeALayout -> addWidget(m_ChangeableARB);

	m_FrozenARB = new QRadioButton(i18n("Frozen"), changeABG);
	changeALayout -> addWidget(m_FrozenARB);

	m_AddOnlyARB = new QRadioButton(i18n("Add only"), changeABG);
	changeALayout -> addWidget(m_AddOnlyARB);

	Uml::Changeability_Type changeability = m_assoc->getChangeabilityA();
	if( changeability == Uml::chg_Changeable )
		m_ChangeableARB -> setChecked( true );
	else if( changeability == Uml::chg_Frozen )
		m_FrozenARB -> setChecked( true );
	else
		m_AddOnlyARB -> setChecked( true );

	// Rolename B
        propsBLayout -> addWidget(new QLabel(i18n("Rolename:"),propsBGB), 0, 0);
	m_pRoleBLE = new QLineEdit(propsBGB);
	propsBLayout -> addWidget(m_pRoleBLE, 0, 1);
	m_pRoleBLE -> setText( m_assoc->getRoleNameB() );

	// Multi B
        propsBLayout -> addWidget(new QLabel(i18n("Multiplicity:"),propsBGB), 1, 0);
	m_pMultiBLE = new QLineEdit(propsBGB);
	propsBLayout -> addWidget(m_pMultiBLE, 1, 1);
	m_pMultiBLE -> setText( m_assoc->getMultiB() );

	// Visibility B

	QHBoxLayout * scopeBLayout = new QHBoxLayout(scopeBBG);
	scopeBLayout -> setMargin(margin);

	m_PublicBRB = new QRadioButton(i18n("Public"), scopeBBG);
	scopeBLayout -> addWidget(m_PublicBRB);

	m_PrivateBRB = new QRadioButton(i18n("Private"), scopeBBG);
	scopeBLayout -> addWidget(m_PrivateBRB);

	m_ProtectedBRB = new QRadioButton(i18n("Protected"), scopeBBG);
	scopeBLayout -> addWidget(m_ProtectedBRB);

	scope = m_assoc->getVisibilityB();
	if( scope == Uml::Public )
		m_PublicBRB -> setChecked( true );
	else if( scope == Uml::Private )
		m_PrivateBRB -> setChecked( true );
	else
		m_ProtectedBRB -> setChecked( true );

	// Changeability B
	QHBoxLayout * changeBLayout = new QHBoxLayout(changeBBG);
	changeBLayout -> setMargin(margin);

	m_ChangeableBRB = new QRadioButton(i18n("Changeable"), changeBBG);
	changeBLayout -> addWidget(m_ChangeableBRB);

	m_FrozenBRB = new QRadioButton(i18n("Frozen"), changeBBG);
	changeBLayout -> addWidget(m_FrozenBRB);

	m_AddOnlyBRB = new QRadioButton(i18n("Add only"), changeBBG);
	changeBLayout -> addWidget(m_AddOnlyBRB);

	changeability = m_assoc->getChangeabilityB();
	if( changeability == Uml::chg_Changeable )
		m_ChangeableBRB -> setChecked( true );
	else if( changeability == Uml::chg_Frozen )
		m_FrozenBRB -> setChecked( true );
	else
		m_AddOnlyBRB -> setChecked( true );

	// Documentation
	//

	// Document A
	QHBoxLayout * docALayout = new QHBoxLayout(docAGB);
	docALayout -> setMargin(margin);
	m_pDocA = new QMultiLineEdit(docAGB);
	docALayout -> addWidget(m_pDocA);
	m_pDocA-> setText(m_assoc-> getRoleADoc());
	// m_pDocA-> setText("<<not implemented yet>>");
	// m_pDocA-> setEnabled(false);
	m_pDocA->setWordWrap(QMultiLineEdit::WidgetWidth);

	// Document B
	QHBoxLayout * docBLayout = new QHBoxLayout(docBGB);
	docBLayout -> setMargin(margin);
	m_pDocB = new QMultiLineEdit(docBGB);
	docBLayout -> addWidget(m_pDocB);
	m_pDocB-> setText(m_assoc-> getRoleBDoc());
	// m_pDocB-> setEnabled(false);
	m_pDocB->setWordWrap(QMultiLineEdit::WidgetWidth);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocRolePage::updateObject() {

	// set props
	m_assoc->setRoleNameA(m_pRoleALE->text());
	m_assoc->setRoleNameB(m_pRoleBLE->text());
	m_assoc->setMultiA(m_pMultiALE->text());
	m_assoc->setMultiB(m_pMultiBLE->text());
	
	if(m_PrivateARB->isChecked())
		m_assoc->setVisibilityA(Uml::Private);
	else if(m_ProtectedARB->isChecked())
		m_assoc->setVisibilityA(Uml::Protected);
	else
		m_assoc->setVisibilityA(Uml::Public);
		
		
	if(m_PrivateBRB->isChecked())
		m_assoc->setVisibilityB(Uml::Private);
	else if(m_ProtectedBRB->isChecked())
		m_assoc->setVisibilityB(Uml::Protected);
	else
		m_assoc->setVisibilityB(Uml::Public);
		
		
	if(m_FrozenARB->isChecked())
		m_assoc->setChangeabilityA(Uml::chg_Frozen);
	else if(m_AddOnlyARB->isChecked())
		m_assoc->setChangeabilityA(Uml::chg_AddOnly);
	else
		m_assoc->setChangeabilityA(Uml::chg_Changeable);
		
		
	if(m_FrozenBRB->isChecked())
		m_assoc->setChangeabilityB(Uml::chg_Frozen);
	else if(m_AddOnlyBRB->isChecked())
		m_assoc->setChangeabilityB(Uml::chg_AddOnly);
	else
		m_assoc->setChangeabilityB(Uml::chg_Changeable);
		
	
	m_assoc->setRoleADoc(m_pDocA->text());
	m_assoc->setRoleBDoc(m_pDocB->text());
}


#include "assocrolepage.moc"

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "classgenpage.h"



ClassGenPage::ClassGenPage(UMLDoc *d, QWidget *parent, UMLObject * o) : QWidget(parent) {
	m_pWidget = 0;
	QString name;
	int margin = fontMetrics().height();
	Uml::UMLObject_Type t = o -> getBaseType();
	m_pUmldoc = d;
	if(t == Uml::ot_Concept)
		name = i18n("Class name");
	else if(t == Uml::ot_Actor)
		name = i18n("Actor name");
	else
		name = i18n("Use Case name");
	setMinimumSize(310,330);
	QVBoxLayout * topLayout = new QVBoxLayout(this);
	topLayout -> setSpacing(6);

	//setup name
	QGridLayout * m_pNameLayout = new QGridLayout(topLayout, 4, 2);
	m_pNameLayout -> setSpacing(6);
	m_pNameL = new QLabel(this);
	m_pNameL -> setText(name);
	m_pNameLayout -> addWidget(m_pNameL, 0, 0);

	m_pClassNameLE = new QLineEdit(this);
	m_pNameLayout -> addWidget(m_pClassNameLE, 0, 1);
	m_pClassNameLE->setFocus();

	m_pStereoTypeLE = 0;
	m_pPackageLE = 0;
	m_pAbstractCB = 0;
	m_pDeconCB = 0;
	if(t == Uml::ot_Concept) {
		m_pStereoTypeL = new QLabel(i18n("Stereotype name"), this);
		m_pNameLayout -> addWidget(m_pStereoTypeL, 1, 0);

		m_pStereoTypeLE = new QLineEdit(this);
		m_pNameLayout -> addWidget(m_pStereoTypeLE, 1, 1);

		m_pStereoTypeLE -> setText(o -> getStereotype());

		m_pPackageL = new QLabel(i18n("Package name"), this);
		m_pNameLayout -> addWidget(m_pPackageL, 2, 0);

		m_pPackageLE = new QLineEdit(this);
		m_pNameLayout -> addWidget(m_pPackageLE, 2, 1);

		m_pPackageLE -> setText(o -> getPackage());

		m_pAbstractCB = new QCheckBox( i18n("Abstract class"), this );
		m_pAbstractCB -> setChecked( o -> getAbstract() );
		m_pNameLayout -> addWidget( m_pAbstractCB, 3, 0 );
	}

	//setup scope
	m_pButtonBG = new QButtonGroup(i18n("Visibility"), this);
	QHBoxLayout * scopeLayout = new QHBoxLayout(m_pButtonBG);
	scopeLayout -> setMargin(margin);
	m_pButtonBG -> setExclusive(true);

	m_pPublicRB = new QRadioButton(i18n("Public"), m_pButtonBG);
	scopeLayout -> addWidget(m_pPublicRB);

	m_pPrivateRB = new QRadioButton(i18n("Private"), m_pButtonBG);
	scopeLayout -> addWidget(m_pPrivateRB);

	m_pProtectedRB = new QRadioButton(i18n("Protected"), m_pButtonBG);
	scopeLayout -> addWidget(m_pProtectedRB);
	topLayout -> addWidget(m_pButtonBG);
	//setup documentation
	m_pDocGB = new QGroupBox(this);
	QHBoxLayout * docLayout = new QHBoxLayout(m_pDocGB);
	docLayout -> setMargin(margin);
	m_pDocGB -> setTitle(i18n("Documentation"));

	m_pDoc = new QMultiLineEdit(m_pDocGB);
	docLayout -> addWidget(m_pDoc);
	topLayout -> addWidget(m_pDocGB);

	m_pObject = o;
	//setup fields
	m_pClassNameLE -> setText(o -> getName());
	m_pDoc-> setText(o -> getDoc());
	Uml::Scope s = o -> getScope();
	if(s == Uml::Public)
		m_pPublicRB->setChecked(true);
	else if(s == Uml::Private)
		m_pPrivateRB->setChecked(true);
	else
		m_pProtectedRB -> setChecked(true);

	///////////
	m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
	//////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassGenPage::ClassGenPage(UMLDoc *d, QWidget *parent, ObjectWidget * o) : QWidget(parent) {
	m_pWidget = o;
	m_pDeconCB = 0;
	m_pMultiCB = 0;
	int margin = fontMetrics().height();
	//int t = o -> getBaseType();
	m_pUmldoc = d;
	setMinimumSize(310,330);
	QGridLayout * topLayout = new QGridLayout(this, 2, 1);
	topLayout -> setSpacing(6);

	//setup name
	QGridLayout * m_pNameLayout = new QGridLayout(topLayout, 3, 2);
	m_pNameLayout -> setSpacing(6);
	m_pNameL = new QLabel(this);
	m_pNameL -> setText(i18n("Class name"));
	m_pNameLayout -> addWidget(m_pNameL, 0, 0);

	m_pClassNameLE = new QLineEdit(this);
	m_pClassNameLE -> setText(o -> getName());
	m_pNameLayout -> addWidget(m_pClassNameLE, 0, 1);

	m_pInstanceL = new QLabel(this);
	m_pInstanceL -> setText(i18n("Instance name"));
	m_pNameLayout -> addWidget(m_pInstanceL, 1, 0);

	m_pInstanceLE = new QLineEdit(this);
	m_pInstanceLE -> setText(o -> getInstanceName());
	m_pNameLayout -> addWidget(m_pInstanceLE, 1, 1);
	UMLView * view = m_pUmldoc -> getCurrentView();

	m_pDrawActorCB = new QCheckBox( i18n( "Draw As Actor" ) , this );
	m_pDrawActorCB -> setChecked( o -> getDrawAsActor() );
	m_pNameLayout -> addWidget( m_pDrawActorCB, 2, 0 );

	if(view -> getType() == Uml::dt_Collaboration) {
		m_pMultiCB = new QCheckBox(i18n("Multiple Instance"), this);
		m_pMultiCB -> setChecked(o -> getMultipleInstance());
		m_pNameLayout -> addWidget(m_pMultiCB, 2,1);
		if( m_pDrawActorCB -> isChecked() )
			m_pMultiCB -> setEnabled( false );
	} else//sequence diagram
	{
		m_pDeconCB = new QCheckBox(i18n("Show Deconstruction"), this);
		m_pDeconCB -> setChecked(o -> getShowDeconstruction());
		m_pNameLayout -> addWidget(m_pDeconCB, 2,1);
	}
	//setup documentation
	m_pDocGB = new QGroupBox(this);
	topLayout -> addWidget(m_pDocGB, 1, 0);
	QHBoxLayout * docLayout = new QHBoxLayout(m_pDocGB);
	docLayout -> setMargin(margin);
	m_pDocGB -> setTitle(i18n("Documentation"));

	m_pDoc = new QMultiLineEdit(m_pDocGB);
	m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
	m_pDoc-> setText(o -> getDoc());
	docLayout -> addWidget(m_pDoc);
	m_pObject = 0;//needs to be set to zero
	if( m_pMultiCB )
		connect( m_pDrawActorCB, SIGNAL( toggled( bool ) ), this, SLOT( slotActorToggled( bool ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassGenPage::~ClassGenPage() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassGenPage::updateObject() {
	if(m_pObject) {
		QString name = m_pClassNameLE -> text();

		m_pObject -> setDoc(m_pDoc -> text());
		if(m_pUmldoc->getCurrentView())
			m_pUmldoc->getCurrentView()->updateNoteWidgets();

		if(m_pStereoTypeLE)
			m_pObject -> setStereotype(m_pStereoTypeLE -> text());
		if(m_pPackageLE)
			m_pObject -> setPackage(m_pPackageLE -> text());
		if( m_pAbstractCB )
			m_pObject -> setAbstract( m_pAbstractCB -> isChecked() );
		//make sure unique name
		UMLObject *o = m_pUmldoc -> findUMLObject(m_pObject -> getBaseType(), name);
		if(o && m_pObject != o) {
			KMessageBox::sorry(this, i18n("The name you have chosen\nis already being used.\nThe name has been reset."),
			                   i18n("ERROR 16: Name is not unique."), false);
			m_pClassNameLE -> setText( m_pObject -> getName() );
		} else
			m_pObject -> setName(name);
		Uml::Scope s;
		if(m_pPublicRB -> isChecked())
			s = Uml::Public;
		else if(m_pPrivateRB -> isChecked())
			s = Uml::Private;
		else
			s = Uml::Protected;
		m_pObject -> setScope(s);
	}//end if m_pObject
	else if(m_pWidget) {
		m_pWidget -> setInstanceName(m_pInstanceLE -> text());
		if(m_pMultiCB)
			m_pWidget -> setMultipleInstance(m_pMultiCB -> isChecked());
		m_pWidget -> setDrawAsActor( m_pDrawActorCB -> isChecked() );
		if( m_pDeconCB )
			m_pWidget -> setShowDeconstruction( m_pDeconCB -> isChecked() );
		QString name = m_pClassNameLE -> text();
		m_pWidget -> setDoc(m_pDoc -> text());
		UMLObject * o = m_pWidget -> getUMLObject();
		UMLObject * old = m_pUmldoc -> findUMLObject(o -> getBaseType(), name);
		if(old && o != old) {
			KMessageBox::sorry(this, i18n("The Name you have chosen\nis already being used.\nThe name has been reset."),
			                   i18n("ERROR 16: Name is not unique."), false);
		} else
			o -> setName(name);
	}
}

void ClassGenPage::slotActorToggled( bool state ) {
	if( m_pMultiCB )
		m_pMultiCB -> setEnabled( !state );
}



#include "classgenpage.moc"

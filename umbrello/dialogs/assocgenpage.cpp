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

#include "assocgenpage.h"

AssocGenPage::AssocGenPage (UMLDoc *d, QWidget *parent, AssociationWidget *assoc)
	: QWidget(parent)
{

	m_pAssociationWidget = assoc;
	m_pWidget = 0;
	m_pTypeLE = 0;
	m_pAssocNameLE = 0;
	m_pUmldoc = d;

	constructWidget();

}

AssocGenPage::~AssocGenPage() {}

void AssocGenPage::constructWidget() {

	// bah. should be in association class, not here -b.t.
	QString types[] = { i18n("Generalization"), i18n("Dependancy"),
			i18n("Aggregation"), i18n("Association"), i18n("Implementation"),
			i18n("Composition"), i18n("Unidirectional Association"), i18n("Realization"),
			i18n("State Transition")
			};

	// general configuration of the GUI
	int margin = fontMetrics().height();
	setMinimumSize(310,330);
	QVBoxLayout * topLayout = new QVBoxLayout(this);
	topLayout -> setSpacing(6);

	// group boxes for name, documentation properties
	QGroupBox *nameGB = new QGroupBox(this);
	QGroupBox *docGB = new QGroupBox(this);
	nameGB -> setTitle(i18n("Properties"));
	docGB -> setTitle(i18n("Documentation"));
	topLayout -> addWidget(nameGB);
	topLayout -> addWidget(docGB);

	QGridLayout * nameLayout = new QGridLayout(nameGB, 2, 2);
	nameLayout -> setSpacing(6);
	nameLayout -> setMargin(margin);

	//Association name
	nameLayout -> addWidget(new QLabel(i18n("Name"),nameGB), 0, 0);
	m_pAssocNameLE = new QLineEdit(nameGB);
	nameLayout -> addWidget(m_pAssocNameLE, 0, 1);
	m_pAssocNameLE->setText( m_pAssociationWidget->getName() );

	// document
	QHBoxLayout * docLayout = new QHBoxLayout(docGB);
	docLayout -> setMargin(margin);

	m_pDoc = new QMultiLineEdit(docGB);
	docLayout -> addWidget(m_pDoc);
	m_pDoc-> setText(m_pAssociationWidget-> getDoc());


	// Association Type
	nameLayout -> addWidget(new QLabel(i18n("Type"), nameGB), 1, 0);
	m_pTypeLE = new QLineEdit(nameGB);
	m_pTypeLE->setEnabled(false);
	nameLayout -> addWidget(m_pTypeLE, 1, 1);

	// set value in association type
        switch ( m_pAssociationWidget-> getAssocType()) {
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
        } //end switch

	m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*
AssocGenPage::AssocGenPage(UMLDoc *d, QWidget *parent, ObjectWidget * o, AssociationWidget *a)
	: QWidget(parent)
{

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

	m_pDrawActorCB = new QCheckBox( i18n( "Draw as actor" ) , this );
	m_pDrawActorCB -> setChecked( o -> getDrawAsActor() );
	m_pNameLayout -> addWidget( m_pDrawActorCB, 2, 0 );

	if(view -> getType() == Uml::dt_Collaboration) {
		m_pMultiCB = new QCheckBox(i18n("Multiple instance"), this);
		m_pMultiCB -> setChecked(o -> getMultipleInstance());
		m_pNameLayout -> addWidget(m_pMultiCB, 2,1);
		if( m_pDrawActorCB -> isChecked() )
			m_pMultiCB -> setEnabled( false );
	} else//sequence diagram
	{
		m_pDeconCB = new QCheckBox(i18n("Show deconstruction"), this);
		m_pDeconCB -> setChecked(o -> getShowDeconstruction());
		m_pNameLayout -> addWidget(m_pDeconCB, 2,1);
	}
	//setup documentation
	docGB = new QGroupBox(this);
	topLayout -> addWidget(docGB, 1, 0);
	QHBoxLayout * docLayout = new QHBoxLayout(docGB);
	docLayout -> setMargin(margin);
	docGB -> setTitle(i18n("Documentation"));

	m_pDoc = new QMultiLineEdit(docGB);
	m_pDoc->setWordWrap(QMultiLineEdit::WidgetWidth);
	m_pDoc-> setText(o -> getDoc());
	docLayout -> addWidget(m_pDoc);
	m_pObject = 0;//needs to be set to zero
	if( m_pMultiCB )
		connect( m_pDrawActorCB, SIGNAL( toggled( bool ) ), this, SLOT( slotActorToggled( bool ) ) );
}
		*/


////////////////////////////////////////////////////////////////////////////////////////////////////
void AssocGenPage::updateObject() {

	if(m_pAssociationWidget) {
		QString name = m_pAssocNameLE -> text();

		m_pAssociationWidget->setName(m_pAssocNameLE->text());
		m_pAssociationWidget->setDoc(m_pDoc->text());


	//	if(m_pUmldoc->getCurrentView())
	//		m_pUmldoc->getCurrentView()->updateNoteWidgets();

	} //end if m_pAssociationWidget
}


#include "assocgenpage.moc"

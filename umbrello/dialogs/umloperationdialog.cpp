/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//qt includes
#include <qlayout.h>
#include <qgroupbox.h>
#include <qlistbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>

//kde includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kbuttonbox.h>
//app includes
#include "../operation.h"
#include "../concept.h"
#include "../interface.h"
#include "../umldoc.h"
#include "../listpopupmenu.h"
#include "parmpropdlg.h"
#include "umloperationdialog.h"

UMLOperationDialog::UMLOperationDialog( QWidget * parent, UMLOperation * pOperation ) : KDialogBase( Plain, i18n("Operation Properties"), Help | Ok | Cancel , Ok, parent, "_UMLOPERATIONDLG_", true, true) {
	m_pOperation = pOperation;
	m_pMenu = 0;
	setupDialog();
}

UMLOperationDialog::~UMLOperationDialog() {}

void UMLOperationDialog::setupDialog() {
	UMLDoc* pDoc = dynamic_cast<UMLDoc*>( m_pOperation->parent()->parent() );
	int margin = fontMetrics().height();
	QVBoxLayout * topLayout = new QVBoxLayout( plainPage() );

	m_pGenGB = new QGroupBox(i18n("General Properties"), plainPage() );
	QGridLayout * genLayout = new QGridLayout(m_pGenGB, 2, 4 );
	genLayout -> setMargin(margin);
	genLayout -> setSpacing(10);

	m_pNameL = new QLabel(i18n("Name:"), m_pGenGB);
	genLayout -> addWidget(m_pNameL, 0, 0 );

	m_pNameLE = new QLineEdit(m_pGenGB );
	m_pNameLE -> setText( m_pOperation -> getName() );
	genLayout -> addWidget(m_pNameLE, 0, 1);

	m_pRtypeL = new QLabel(i18n("Type:"), m_pGenGB );
	genLayout -> addWidget(m_pRtypeL, 0, 2);

	m_pRtypeCB = new QComboBox(m_pGenGB );
	genLayout -> addWidget(m_pRtypeCB, 0, 3);

	m_pAbstractCB = new QCheckBox( i18n("&Abstract operation"), m_pGenGB );
	m_pAbstractCB -> setChecked( m_pOperation -> getAbstract() );
	m_pStaticCB = new QCheckBox( i18n("Classifier &scope (\"static\")"), m_pGenGB );
	m_pStaticCB -> setChecked( m_pOperation -> getStatic() );
	genLayout -> addWidget( m_pAbstractCB, 1, 0 );
	genLayout -> addWidget( m_pStaticCB, 1, 1 );
	topLayout -> addWidget( m_pGenGB );


	m_pScopeBG = new QButtonGroup(i18n("Visibility"), plainPage() );

	QHBoxLayout * scopeLayout = new QHBoxLayout(m_pScopeBG);
	scopeLayout -> setMargin(margin);

	m_pPublicRB = new QRadioButton(i18n("&Public"), m_pScopeBG);
	scopeLayout -> addWidget(m_pPublicRB);

	m_pPrivateRB = new QRadioButton(i18n("P&rivate"), m_pScopeBG);
	scopeLayout -> addWidget(m_pPrivateRB);

	m_pProtectedRB = new QRadioButton(i18n("Pro&tected"), m_pScopeBG);
	scopeLayout -> addWidget(m_pProtectedRB);
	topLayout -> addWidget(m_pScopeBG);

	m_pParmsGB = new QGroupBox(i18n("Parameters"), plainPage() );
	QVBoxLayout * parmsLayout = new QVBoxLayout(m_pParmsGB);
	parmsLayout -> setMargin(margin);
	parmsLayout->setSpacing(10);

	m_pParmsLB = new QListBox(m_pParmsGB);
	parmsLayout -> addWidget(m_pParmsLB);

	KButtonBox* buttonBox = new KButtonBox(m_pParmsGB);
	buttonBox->addButton( i18n("&New Parameter..."), this, SLOT(slotNewParameter()) );
	m_pDeleteButton = buttonBox->addButton( i18n("&Delete"), this, SLOT(slotDeleteParameter()) );
	m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), this,
						    SLOT(slotParameterProperties()) );
	parmsLayout->addWidget(buttonBox);

	topLayout -> addWidget(m_pParmsGB);

	m_pDeleteButton->setEnabled(false);
	m_pPropertiesButton->setEnabled(false);

	//add some standard return types to combo box
	QString types[] = {i18n("void"), i18n("int"), i18n("long"), i18n("bool"), i18n("string"), i18n("double"),
	                   i18n("float"), i18n("date")};

	for (int i=0; i<6; i++) {
		m_pRtypeCB->insertItem(types[i]);
	}

	m_pRtypeCB->setDuplicatesEnabled(false);//only allow one of each type in box
	m_pRtypeCB->setEditable(true);
	m_pRtypeCB->setAutoCompletion(true);

	//now add the Concepts and Interfaces
	QPtrList<UMLConcept> namesList( pDoc->getConcepts() );
	UMLConcept* pConcept = 0;
	for(pConcept=namesList.first(); pConcept!=0 ;pConcept=namesList.next()) {
		m_pRtypeCB->insertItem( pConcept->getName() );
	}
	QPtrList<UMLInterface> interfaceList( pDoc->getInterfaces() );
	UMLInterface* pInterface = 0;
	for(pInterface=interfaceList.first(); pInterface!=0 ;pInterface=interfaceList.next()) {
		m_pRtypeCB->insertItem( pInterface->getName() );
	}

	//work out which one to select
	int returnBoxCount = 0;
	bool foundReturnType = false;
	while (returnBoxCount < m_pRtypeCB->count() && foundReturnType == false) {
		QString returnBoxString = m_pRtypeCB->text(returnBoxCount);
		if ( returnBoxString == m_pOperation->getReturnType() ) {
			foundReturnType = true;
			m_pRtypeCB->setCurrentItem(returnBoxCount);
		} else {
			returnBoxCount++;
		}
	}

	if (!foundReturnType) {
		m_pRtypeCB->insertItem( m_pOperation->getReturnType(), 0 );
		m_pRtypeCB->setCurrentItem(0);
	}

	//fill in parm list box
	QList<UMLAttribute> * list = m_pOperation -> getParmList();
	UMLAttribute * pAtt = 0;
	for( pAtt = list -> first(); pAtt != 0; pAtt = list -> next() )
		m_pParmsLB -> insertItem( pAtt -> getName() );

	//set scope
	Uml::Scope scope = m_pOperation -> getScope();
	if( scope == Uml::Public )
		m_pPublicRB -> setChecked( true );
	else if( scope == Uml::Private )
		m_pPrivateRB -> setChecked( true );
	else
		m_pProtectedRB -> setChecked( true );

	//setup parm list box signals
	connect(m_pParmsLB, SIGNAL(clicked(QListBoxItem*)),
		this, SLOT(slotParamsBoxClicked(QListBoxItem*)));

	connect(m_pParmsLB, SIGNAL(rightButtonPressed(QListBoxItem *, const QPoint &)),
	        this, SLOT(slotParmRightButtonPressed(QListBoxItem *, const QPoint &)));

	connect(m_pParmsLB, SIGNAL(rightButtonClicked(QListBoxItem *, const QPoint &)),
	        this, SLOT(slotParmRightButtonClicked(QListBoxItem *, const QPoint &)));


	connect(m_pParmsLB, SIGNAL(doubleClicked(QListBoxItem *)),
	        this, SLOT(slotParmDoubleClick(QListBoxItem *)));

	m_pNameLE->setFocus();
}

void UMLOperationDialog::slotParmRightButtonPressed(QListBoxItem *item, const QPoint &p) {
	ListPopupMenu::Menu_Type type = ListPopupMenu::mt_Undefined;
	if(item)//pressed on an item
	{
		type = ListPopupMenu::mt_Parameter_Selected;
	} else//pressed into fresh air
	{
		type = ListPopupMenu::mt_New_Parameter;
	}
	if(m_pMenu) {
		m_pMenu -> hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotParmPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
	m_pMenu = new ListPopupMenu(this, type);
	m_pMenu->popup(p);
	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotParmPopupMenuSel(int)));

}

void UMLOperationDialog::slotParmRightButtonClicked(QListBoxItem */*item*/, const QPoint &/*p*/) {
	if(m_pMenu) {
		m_pMenu -> hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotParmPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}

void UMLOperationDialog::slotParmDoubleClick(QListBoxItem *item) {
	if(!item)
		return;
	slotParmPopupMenuSel(ListPopupMenu::mt_Properties);
}

void UMLOperationDialog::slotParmPopupMenuSel(int id) {
	if( id == ListPopupMenu::mt_Rename || id == ListPopupMenu::mt_Properties ) {
		slotParameterProperties();
	} else if( id == ListPopupMenu::mt_New_Parameter ) {
		slotNewParameter();
	}
	else if( id == ListPopupMenu::mt_Delete ) {
		slotDeleteParameter();
	}
}

void UMLOperationDialog::slotNewParameter() {
	int result = 0;
	UMLAttribute* pAtt = 0;
	UMLDoc* pDoc = dynamic_cast<UMLDoc *>( m_pOperation->parent()->parent() );

	QString currentName = m_pOperation->getUniqueParameterName();
	UMLAttribute* newAttribute = new UMLAttribute(0, currentName, 0);

	ParmPropDlg dlg(this, pDoc, newAttribute);
	result = dlg.exec();
	QString name = dlg.getName();
	pAtt = m_pOperation -> findParm( name );
	if( result ) {
		if( name.length() == 0 ) {
			KMessageBox::error(this, i18n("You have entered an invalid parameter name."),
					   i18n("Parameter Name Invalid"), false);
			return;
		}
		if( !pAtt ) {
			m_pOperation->addParm( dlg.getTypeName(), name,
					       dlg.getInitialValue(), dlg.getDoc() );
			m_pParmsLB -> insertItem( dlg.getName() );
			pDoc -> setModified( true );
		} else {
			KMessageBox::sorry(this, i18n("The parameter name you have chosen\nis already being used in this operation."),
					   i18n("Parameter Name Not Unique"), false);
		}
	}//end if result
	delete newAttribute;
}

void UMLOperationDialog::slotDeleteParameter() {
	UMLAttribute* pOldAtt = m_pOperation->findParm( m_pParmsLB->currentText() );
	UMLDoc* pDoc = dynamic_cast<UMLDoc*>( m_pOperation->parent()->parent() );

	m_pOperation->removeParm( pOldAtt );
	m_pParmsLB->removeItem( m_pParmsLB->currentItem() );
	pDoc->setModified(true);

	m_pDeleteButton->setEnabled(false);
	m_pPropertiesButton->setEnabled(false);
}

void UMLOperationDialog::slotParameterProperties() {
	int result = 0;
	UMLAttribute* pAtt = 0, * pOldAtt = 0;
	pOldAtt = m_pOperation->findParm( m_pParmsLB->currentText() );
	UMLDoc* pDoc = dynamic_cast<UMLDoc*>( m_pOperation->parent()->parent() );

	if( !pOldAtt ) {
		kdDebug() << "THE impossible has occurred for:" << m_pParmsLB->currentText() << endl;
		return;
	}//should never occur
	ParmPropDlg dlg(this, pDoc, pOldAtt);
	result = dlg.exec();
	QString name = dlg.getName();
	pAtt = m_pOperation->findParm( name );
	if( result ) {
		if( name.length() == 0 ) {
			KMessageBox::error(this, i18n("You have entered an invalid parameter name."),
					   i18n("Parameter Name Invalid"), false);
			return;
		}
		if( !pAtt || ( pOldAtt->getTypeName() != dlg.getTypeName() ) ) {
			pOldAtt->setName( name );
			pOldAtt->setTypeName( dlg.getTypeName() );
			m_pParmsLB->changeItem( dlg.getName(), m_pParmsLB -> currentItem() );
			pOldAtt->setDoc( dlg.getDoc() );
			pOldAtt->setInitialValue( dlg.getInitialValue() );
			pDoc->setModified( true );
		} else if( pAtt != pOldAtt ) {
			KMessageBox::error(this, i18n("The parameter name you have chosen is already being used in this operation."),
					   i18n("Parameter Name Not Unique."), false);
		}
	}
}

void UMLOperationDialog::slotParamsBoxClicked(QListBoxItem* parameterItem) {
	if (parameterItem) {
		m_pDeleteButton->setEnabled(true);
		m_pPropertiesButton->setEnabled(true);
	} else {
		m_pDeleteButton->setEnabled(false);
		m_pPropertiesButton->setEnabled(false);
	}
}

bool UMLOperationDialog::apply() {
	QString name = m_pNameLE -> text();

	if( m_pPublicRB -> isChecked() )
		m_pOperation -> setScope( Uml::Public );
	else if( m_pPrivateRB -> isChecked() )
		m_pOperation -> setScope( Uml::Private );
	else
		m_pOperation -> setScope( Uml::Protected );
	m_pOperation -> setReturnType( m_pRtypeCB -> currentText() );
	m_pOperation -> setAbstract( m_pAbstractCB -> isChecked() );
	m_pOperation -> setStatic( m_pStaticCB -> isChecked() );

	QPtrList<UMLObject> list;
	if (static_cast<UMLObject*>(m_pOperation->parent())->getBaseType() == Uml::ot_Concept) {
		UMLConcept* pConcept = static_cast<UMLConcept*>( m_pOperation->parent() );
		list= pConcept->findChildObject(Uml::ot_Operation, name);
	} else if (static_cast<UMLObject*>(m_pOperation->parent())->getBaseType() == Uml::ot_Interface) {
		UMLInterface* pInterface = static_cast<UMLInterface*>( m_pOperation->parent() );
		list = pInterface->findChildObject(Uml::ot_Operation, name);
	} else {
		kdWarning() << "not a concept or interface" << endl;
	}

	if( name.length() == 0 ) {
		KMessageBox::error(this, i18n("You have entered an invalid operation name."),
		                   i18n("Operation Name Invalid"), false);
		m_pNameLE -> setText( m_pOperation -> getName() );
		return false;
	}
	if( list.count() != 0 && list.findRef( m_pOperation ) ) {
		if( KMessageBox::warningYesNo( this , i18n( "The name you entered was not unique!\nIs this what you wanted?" ),
		                               i18n( "Name Not unique" ) ) == KMessageBox::No ) {
			m_pNameLE -> setText( m_pOperation -> getName() );
			return false;
		}
	}
	m_pOperation -> setName( name );
	return true;
}

void UMLOperationDialog::slotApply() {
	apply();
}

void UMLOperationDialog::slotOk() {
	if ( apply() ) {
		accept();
	}
}







#include "umloperationdialog.moc"

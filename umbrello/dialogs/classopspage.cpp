/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classopspage.h"
#include "../operation.h"
#include "../umldoc.h"
#include "umloperationdialog.h"
#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>

ClassOpsPage::ClassOpsPage(QWidget *parent, UMLClassifier * c, UMLDoc * doc) : QWidget(parent) {
	m_bSigWaiting = false;
	this -> m_pDoc = doc;
	m_pConcept = c;
	m_pMenu = 0;
	int margin = fontMetrics().height();

	QVBoxLayout * mainLayout = new QVBoxLayout( this );
	mainLayout -> setSpacing(10);

	m_pOpsGB = new QGroupBox(i18n("Operations"), this );
	QGridLayout* opsLayout = new QGridLayout(m_pOpsGB, 2, 2);
	opsLayout -> setMargin(margin);
	opsLayout -> setSpacing( 10 );

	m_pOpsLB = new QListBox(m_pOpsGB);
	opsLayout -> addWidget(m_pOpsLB, 0, 0);

	QVBoxLayout* buttonLayout = new QVBoxLayout( opsLayout );
	m_pUpArrowB = new KArrowButton( m_pOpsGB );
	m_pUpArrowB -> setEnabled( false );
	buttonLayout -> addWidget( m_pUpArrowB );

	m_pDownArrowB = new KArrowButton( m_pOpsGB, Qt::DownArrow );
	m_pDownArrowB -> setEnabled( false );
	buttonLayout -> addWidget( m_pDownArrowB );

	KButtonBox* buttonBox = new KButtonBox(m_pOpsGB);
	buttonBox->addButton( i18n("N&ew Operation..."), this, SLOT(slotNewOperation()) );
	m_pDeleteOperationButton = buttonBox->addButton( i18n("&Delete"),
							  this, SLOT(slotDelete()) );
	m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), this, SLOT(slotProperties()) );
	opsLayout->addMultiCellWidget(buttonBox, 1, 1, 0, 1);

	mainLayout -> addWidget(m_pOpsGB);

	m_pDocGB = new QGroupBox( i18n( "Documentation" ), this );
	QHBoxLayout * docLayout = new QHBoxLayout( m_pDocGB );
	docLayout -> setMargin( margin );
	docLayout -> setSpacing( 10 );

	m_pDocTE = new QTextEdit( m_pDocGB );
	docLayout -> addWidget( m_pDocTE );
	mainLayout -> addWidget( m_pDocGB );

	UMLObject *obj;
	m_pOpList = m_pConcept -> getOpList();
	for( obj = m_pOpList -> first(); obj != 0; obj = m_pOpList -> next() )
		m_pOpsLB -> insertItem( obj -> getName() );
	m_pOldOp = 0;
	//setup op list box signals
	connect(m_pOpsLB, SIGNAL(clicked(QListBoxItem *)), this, SLOT(slotClicked(QListBoxItem *)));
	connect(m_pOpsLB, SIGNAL(rightButtonPressed(QListBoxItem *, const QPoint &)),
	        this, SLOT(slotOpRightButtonPressed(QListBoxItem *, const QPoint &)));
	enableWidgets(false);
	connect(m_pDoc, SIGNAL(sigObjectCreated(UMLObject *)), this, SLOT(slotOperationCreated(UMLObject *)));
//FIXME	connect( c, SIGNAL(modified( )), this, SLOT(slotOperationRenamed(UMLObject *)));
	connect( m_pUpArrowB, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );
	connect( m_pDownArrowB, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );
	connect( m_pOpsLB, SIGNAL( doubleClicked( QListBoxItem * ) ), this, SLOT( slotDoubleClick( QListBoxItem * ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassOpsPage::~ClassOpsPage() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassOpsPage::slotClicked(QListBoxItem *i) {
	if( !i ) {
		enableWidgets(false);
		m_pOldOp = 0;
		m_pOpsLB -> clearSelection();
		return;
	}
	QString name = m_pOpsLB -> currentText();
	UMLOperation * o = m_pOpList -> at( m_pOpsLB -> index( i ) );

	//if not first time an item is highlighted
	//save old highlighted item first
	if( m_pOldOp ) {
		m_pOldOp -> setDoc( m_pDocTE -> text() );
	}//end if
	m_pDocTE -> setText( o -> getDoc() );
	enableWidgets(true);
	m_pOldOp = o;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassOpsPage::enableWidgets( bool state ) {
	m_pDocTE -> setEnabled( state );
	if( !state ) {
		m_pDocTE -> setText( "" );
		m_pUpArrowB -> setEnabled( false );
		m_pDownArrowB -> setEnabled( false );
		m_pDeleteOperationButton->setEnabled(false);
		m_pPropertiesButton->setEnabled(false);
		return;
	}
	/*now check the order buttons.
		Double check an item is selected
	   If only one att. in list make sure there disabled.
		If at top item,only allow down arrow to be enabled.
		If at bottom item. only allow up arrow to be enabled.
	*/
	int index = m_pOpsLB -> currentItem();
	if( m_pOpsLB -> count() == 1 || index == -1 ) {
		m_pUpArrowB -> setEnabled( false );
		m_pDownArrowB -> setEnabled( false );
	} else if( index == 0 ) {
		m_pUpArrowB -> setEnabled( false );
		m_pDownArrowB -> setEnabled( true );
	} else if( index == (int)m_pOpsLB -> count() - 1 ) {
		m_pUpArrowB -> setEnabled( true );
		m_pDownArrowB -> setEnabled( false );
	} else {
		m_pUpArrowB -> setEnabled( true );
		m_pDownArrowB -> setEnabled( true );
	}
	m_pDeleteOperationButton->setEnabled(true);
	m_pPropertiesButton->setEnabled(true);
}

void ClassOpsPage::updateObject() {
	QListBoxItem *i = m_pOpsLB -> item(m_pOpsLB -> currentItem());
	slotClicked(i);
	QStringList stringList;
	int count = m_pOpsLB -> count();
	for( int j = 0; j < count ; j++ )
		stringList.append( m_pOpsLB -> text( j ) );
	/*	if( count > 0 )
			m_pConcept -> reorderOperations( stringList );*/
}

void ClassOpsPage::slotOpRightButtonPressed(QListBoxItem *item, const QPoint &p) {

	//change for operation
	ListPopupMenu::Menu_Type type = ListPopupMenu::mt_Undefined;
	if(item)//pressed on an item
	{
		type = ListPopupMenu::mt_Operation_Selected;
	} else//pressed into fresh air
	{
		type = ListPopupMenu::mt_New_Operation;
	}
	if(m_pMenu) {
		m_pMenu -> hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotOpPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
	m_pMenu = new ListPopupMenu(this, type);
	m_pMenu->popup(p);
	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotOpPopupMenuSel(int)));
}

void ClassOpsPage::slotOpRightButtonClicked(QListBoxItem * /*item*/, const QPoint & /*p*/) {
	if(m_pMenu) {
		m_pMenu -> hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotOpPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}

void ClassOpsPage::slotOpPopupMenuSel(int id) {
	UMLOperation * o = m_pOpList -> at( m_pOpsLB -> currentItem() );
	if(!o && id != ListPopupMenu::mt_New_Operation) {
		kdDebug() << "can't find op from selection" << endl;
		return;
	}
	switch(id) {
		case ListPopupMenu::mt_New_Operation:
			slotNewOperation();
			break;

		case ListPopupMenu::mt_Delete:
			slotDelete();
			break;

		case ListPopupMenu::mt_Rename:
			m_bSigWaiting = true;
			m_pDoc -> renameChildUMLObject(o);
			break;

		case ListPopupMenu::mt_Properties:
			slotProperties();
			break;
	}
}

void ClassOpsPage::slotOperationCreated(UMLObject * object) {
	if(!m_bSigWaiting)
		return;
	if( object->getBaseType() != Uml::ot_Operation ||
	    dynamic_cast<UMLClassifier*>(object->parent()) != m_pConcept)
	{
		return;
	}
	int index = m_pOpsLB -> count();
	m_pOpsLB ->insertItem(object -> getName(), index);
	m_bSigWaiting = false;
	slotClicked(0);
}

void ClassOpsPage::slotOperationRenamed(UMLObject * object) {
	if(!m_bSigWaiting)
		return;
	int index = m_pOpsLB -> currentItem();
	m_pOpsLB -> changeItem(object -> getName(), index);
	m_bSigWaiting = false;
}

void ClassOpsPage::slotUpClicked() {
	int count = m_pOpsLB -> count();
	int index = m_pOpsLB -> currentItem();
	//shouldn't occur, but just in case
	if( count <= 1 || index <= 0 )
		return;

	//swap the text around ( meaning operation )
	QString aboveString = m_pOpsLB -> text( index - 1 );
	QString currentString = m_pOpsLB -> text( index );
	m_pOpsLB -> changeItem( currentString, index -1 );
	m_pOpsLB -> changeItem( aboveString, index );
	//set the moved atttribute selected
	QListBoxItem * item = m_pOpsLB -> item( index - 1 );
	m_pOpsLB -> setSelected( item, true );
	//now change around in the list
	UMLOperation * aboveOp = m_pOpList -> at( index - 1 );
	UMLOperation * currentOp = m_pOpList -> take( index );
	m_pOpList -> insert( m_pOpList -> findRef( aboveOp ), currentOp );
	slotClicked( item );
}

void ClassOpsPage::slotDownClicked() {
	int count = m_pOpsLB -> count();
	int index = m_pOpsLB -> currentItem();
	//shouldn't occur, but just in case
	if( count <= 1 || index >= count - 1 )
		return;

	//swap the text around ( meaning attributes )
	QString belowString = m_pOpsLB -> text( index + 1 );
	QString currentString = m_pOpsLB -> text( index );
	m_pOpsLB -> changeItem( currentString, index + 1 );
	m_pOpsLB -> changeItem( belowString, index );
	//set the moved atttribute selected
	QListBoxItem * item = m_pOpsLB -> item( index + 1 );
	m_pOpsLB -> setSelected( item, true );
	//now change around in the list
	UMLOperation * belowOp = m_pOpList -> at( index + 1 );
	UMLOperation * currentOp = m_pOpList -> take( index );
	m_pOpList -> insert( m_pOpList -> findRef( belowOp ) + 1, currentOp );
	slotClicked( item );
}

void ClassOpsPage::slotDoubleClick( QListBoxItem * item ) {
	if( !item )
		return;
	UMLOperation * pOp = m_pOpList -> at( m_pOpsLB -> index( item ) );
	if( !pOp ) {
		kdDebug() << "can't find op from selection" << endl;
		return;
	}
	UMLOperationDialog dlg( this, pOp );
	if( dlg.exec() ) {
		m_pOpsLB->changeItem( pOp->getName(), m_pOpsLB->index(item) );
//FIXME		m_pDoc->signalChildUMLObjectUpdate(pOp);
	}
}

void ClassOpsPage::slotProperties() {
	//save highlighted item first
	UMLOperation* selectedOperation = m_pOpList->at( m_pOpsLB -> currentItem() );
	if (selectedOperation)
		selectedOperation -> setDoc( m_pDocTE -> text() );

	slotDoubleClick( m_pOpsLB -> item( m_pOpsLB -> currentItem() ) );
}

void ClassOpsPage::slotDelete() {
	UMLOperation* selectedOperation = m_pOpList->at( m_pOpsLB -> currentItem() );
	//should really wait for signal back
	//but really shouldn't matter
	m_pDoc->removeUMLObject(selectedOperation);
	m_pOpsLB->removeItem( m_pOpsLB->currentItem() );
	slotClicked(0);
}

void ClassOpsPage::slotNewOperation() {
	//save highlighted item first
	UMLOperation* selectedOperation = m_pOpList->at( m_pOpsLB -> currentItem() );
	if (selectedOperation)
		selectedOperation -> setDoc( m_pDocTE -> text() );

	m_bSigWaiting = true;
	m_pDoc->createUMLObject(m_pConcept, Uml::ot_Operation);
}

#include "classopspage.moc"

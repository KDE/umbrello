/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classattpage.h"
#include "umlattributedialog.h"
#include "../attribute.h"
#include "../umldoc.h"
#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>

ClassAttPage::ClassAttPage(QWidget *parent, UMLClass * c, UMLDoc * doc) : QWidget(parent) {
	m_bSigWaiting = false;
	m_pDoc = doc;
	m_pClass = c;
	m_pMenu = 0;
	int margin = fontMetrics().height();
	setMinimumSize(310,330);

	QVBoxLayout* mainLayout = new QVBoxLayout( this );
	mainLayout -> setSpacing(10);

	m_pAttsGB = new QGroupBox(i18n("Attributes"), this );
	QGridLayout* attsLayout = new QGridLayout( m_pAttsGB, 2, 2 );
	attsLayout -> setMargin(margin);
	attsLayout -> setSpacing ( 10 );

	m_pAttsLB = new QListBox(m_pAttsGB );
	attsLayout -> addWidget(m_pAttsLB, 0, 0);

	QVBoxLayout* buttonLayout = new QVBoxLayout( attsLayout );
	m_pUpArrowB = new KArrowButton( m_pAttsGB );
	m_pUpArrowB -> setEnabled( false );
	buttonLayout -> addWidget( m_pUpArrowB );

	m_pDownArrowB = new KArrowButton( m_pAttsGB, Qt::DownArrow );
	m_pDownArrowB -> setEnabled( false );
	buttonLayout -> addWidget( m_pDownArrowB );

	KButtonBox* buttonBox = new KButtonBox(m_pAttsGB);
	buttonBox->addButton( i18n("N&ew Attribute..."), this, SLOT(slotNewAttribute()) );
	m_pDeleteAttributeButton = buttonBox->addButton( i18n("&Delete"),
							  this, SLOT(slotDelete()) );
	m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), this, SLOT(slotProperties()) );
	attsLayout->addMultiCellWidget(buttonBox, 1, 1, 0, 1);

	mainLayout -> addWidget(m_pAttsGB);

	m_pDocGB = new QGroupBox(i18n("Documentation"), this);
	QVBoxLayout* docLayout = new QVBoxLayout( m_pDocGB );
	m_pDocTE = new QTextEdit( m_pDocGB );
	docLayout -> setMargin(margin);
	docLayout -> setSpacing ( 10 );
	docLayout -> addWidget( m_pDocTE );
	mainLayout -> addWidget(m_pDocGB);

	//add attributes to list
	UMLAttribute *a;
	m_pAttList = c -> getAttList();
	for(a=m_pAttList->first();a != 0;a=m_pAttList->next())
	{
		m_pAttsLB -> insertItem(a->getName());
		connect( a, SIGNAL(modified()),this,SLOT(attributeModified()));
	}

	enableWidgets(false);//disable widgets until an att is chosen
	m_pOldAtt = 0;
	connect(m_pAttsLB, SIGNAL(clicked(QListBoxItem *)), this, SLOT(slotClicked(QListBoxItem *)));

	connect(m_pAttsLB, SIGNAL(rightButtonPressed(QListBoxItem *, const QPoint &)),
	        this, SLOT(slotRightButtonPressed(QListBoxItem *, const QPoint &)));

	connect(m_pAttsLB, SIGNAL(rightButtonClicked(QListBoxItem *, const QPoint &)),
	        this, SLOT(slotRightButtonClicked(QListBoxItem *, const QPoint &)));
	connect(m_pDoc, SIGNAL(sigObjectCreated(UMLObject *)), this, SLOT(slotAttributeCreated(UMLObject *)));

	connect( m_pUpArrowB, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );
	connect( m_pDownArrowB, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );
	connect( m_pAttsLB, SIGNAL( doubleClicked( QListBoxItem * ) ), this, SLOT( slotDoubleClick( QListBoxItem * ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassAttPage::~ClassAttPage() {

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassAttPage::enableWidgets(bool state) {
	m_pDocTE -> setEnabled( state );
	//if disabled clear contents
	if( !state ) {
		m_pDocTE -> setText( "" );
		m_pUpArrowB -> setEnabled( false );
		m_pDownArrowB -> setEnabled( false );
		m_pDeleteAttributeButton->setEnabled(false);
		m_pPropertiesButton->setEnabled(false);
		return;
	}
	/*now check the order buttons.
		Double check an item is selected
	   If only one att. in list make sure there disabled.
		If at top item,only allow down arrow to be enabled.
		If at bottom item. only allow up arrow to be enabled.
	*/
	int index = m_pAttsLB -> currentItem();
	if( m_pAttsLB -> count() == 1 || index == -1 ) {
		m_pUpArrowB -> setEnabled( false );
		m_pDownArrowB -> setEnabled( false );
	} else if( index == 0 ) {
		m_pUpArrowB -> setEnabled( false );
		m_pDownArrowB -> setEnabled( true );
	} else if( index == (int)m_pAttsLB -> count() - 1 ) {
		m_pUpArrowB -> setEnabled( true );
		m_pDownArrowB -> setEnabled( false );
	} else {
		m_pUpArrowB -> setEnabled( true );
		m_pDownArrowB -> setEnabled( true );
	}
	m_pDeleteAttributeButton->setEnabled(true);
	m_pPropertiesButton->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassAttPage::slotClicked(QListBoxItem *item) {
	//make sure clicked on an item
	if(!item) {
		enableWidgets(false);
		m_pOldAtt = 0;
		m_pAttsLB -> clearSelection();
		return;
	}

	QString name = m_pAttsLB ->currentText();
	UMLAttribute * pAtt = m_pAttList -> at( m_pAttsLB -> index( item ) );
	//if not first time an item is highlighted
	//save old highlighted item first
	if(m_pOldAtt) {
		m_pOldAtt -> setDoc( m_pDocTE -> text() );
	}//end if old att
	//now update screen
	m_pDocTE -> setText( pAtt -> getDoc() );
	enableWidgets(true);
	m_pOldAtt = pAtt;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassAttPage::updateObject() {
	QListBoxItem *i = m_pAttsLB -> item(m_pAttsLB -> currentItem());
	slotClicked(i);
	QStringList stringList;
	int count = m_pAttsLB -> count();
	for( int j = 0; j < count ; j++ )
		stringList.append( m_pAttsLB -> text( j ) );
	/*	if( count > 0 )
			m_pClass -> reorderAttributes( stringList );             */
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassAttPage::slotAttributeCreated(UMLObject * object) {
	if(!m_bSigWaiting) {
		return;
	}
	if( object->getBaseType() != Uml::ot_Attribute ||
	    object->parent() != m_pClass )
	{
		return;
	}
	int index = m_pAttsLB -> count();
	m_pAttsLB ->insertItem(object -> getName(), index);
	m_bSigWaiting = false;
	slotClicked(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassAttPage::attributeModified( ) {
	if(!m_bSigWaiting) {
		return;
	}
	//is this safe???
	UMLObject *object = const_cast<UMLObject*>(dynamic_cast<const UMLObject*>(sender()));
	int index = m_pAttsLB -> currentItem();
	m_pAttsLB -> changeItem(object -> getName(), index);
	m_bSigWaiting = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassAttPage::slotRightButtonClicked(QListBoxItem * /*item*/, const QPoint &/* p*/) {
	if(m_pMenu) {
		m_pMenu -> hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassAttPage::slotRightButtonPressed(QListBoxItem * item, const QPoint & p)
{
	ListPopupMenu::Menu_Type type = ListPopupMenu::mt_Undefined;
	if(item)//pressed on an item
	{
		type = ListPopupMenu::mt_Attribute_Selected;
	} else//pressed into fresh air
	{
		type = ListPopupMenu::mt_New_Attribute;
	}
	if(m_pMenu) {
		m_pMenu -> hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
	m_pMenu = new ListPopupMenu(this, type);


	m_pMenu->popup(p);
	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassAttPage::slotPopupMenuSel(int id) {
	UMLAttribute * a = m_pAttList -> at( m_pAttsLB -> currentItem() );
	if(!a && id != ListPopupMenu::mt_New_Attribute) {
		kdDebug() << "can't find att from selection" << endl;
		return;
	}
	switch(id) {
		case ListPopupMenu::mt_New_Attribute:
			slotNewAttribute();
			break;

		case ListPopupMenu::mt_Delete:
			slotDelete();
			break;

		case ListPopupMenu::mt_Rename:
			m_bSigWaiting = true;
			m_pDoc -> renameChildUMLObject(a);
			break;

		case ListPopupMenu::mt_Properties:
			slotProperties();
			break;
	}
}

void ClassAttPage::slotUpClicked() {
	int count = m_pAttsLB -> count();
	int index = m_pAttsLB -> currentItem();
	//shouldn't occur, but just in case
	if( count <= 1 || index <= 0 )
		return;

	//swap the text around ( meaning attributes )
	QString aboveString = m_pAttsLB -> text( index - 1 );
	QString currentString = m_pAttsLB -> text( index );
	m_pAttsLB -> changeItem( currentString, index -1 );
	m_pAttsLB -> changeItem( aboveString, index );
	//set the moved atttribute selected
	QListBoxItem * item = m_pAttsLB -> item( index - 1 );
	m_pAttsLB -> setSelected( item, true );
	//now change around in the list
	UMLAttribute * aboveAtt = m_pAttList -> at( index - 1 );
	UMLAttribute * currentAtt = m_pAttList -> take( index );
	m_pAttList -> insert( m_pAttList -> findRef( aboveAtt ), currentAtt );
	slotClicked( item );
}

void ClassAttPage::slotDownClicked() {
	int count = m_pAttsLB -> count();
	int index = m_pAttsLB -> currentItem();
	//shouldn't occur, but just in case
	if( count <= 1 || index >= count - 1 )
		return;

	//swap the text around ( meaning attributes )
	QString belowString = m_pAttsLB -> text( index + 1 );
	QString currentString = m_pAttsLB -> text( index );
	m_pAttsLB -> changeItem( currentString, index + 1 );
	m_pAttsLB -> changeItem( belowString, index );
	//set the moved atttribute selected
	QListBoxItem * item = m_pAttsLB -> item( index + 1 );
	m_pAttsLB -> setSelected( item, true );
	//now change around in the list
	UMLAttribute * aboveAtt = m_pAttList -> at( index + 1 );
	UMLAttribute * currentAtt = m_pAttList -> take( index );
	m_pAttList -> insert( m_pAttList -> findRef( aboveAtt ) + 1, currentAtt );
	slotClicked( item );
}

void ClassAttPage::slotDoubleClick( QListBoxItem * item ) {
	if( !item )
		return;
	QString name = item -> text();
	UMLAttribute * pAtt  = m_pAttList -> at( m_pAttsLB -> index( item ) );
	if( !pAtt ) {
		kdDebug() << "can't find att from selection" << endl;
		return;
	}
	UMLAttributeDialog dlg( this, pAtt );
	if( dlg.exec() ) {
		m_pAttsLB->changeItem( pAtt->getName(), m_pAttsLB->index(item) );

	}
}

void ClassAttPage::slotDelete() {
	UMLAttribute* selectedAttribute = m_pAttList->at( m_pAttsLB->currentItem() );
	//should really wait for signal back
	//but really shouldn't matter
	m_pDoc->removeUMLObject(selectedAttribute);
	m_pAttsLB->removeItem( m_pAttsLB->currentItem());
	slotClicked(0);
}

void ClassAttPage::slotProperties() {
	//save highlighted item first
	UMLAttribute* selectedAttribute = m_pAttList->at( m_pAttsLB->currentItem() );
	if (selectedAttribute)
		selectedAttribute -> setDoc( m_pDocTE -> text() );

	slotDoubleClick( m_pAttsLB->item( m_pAttsLB->currentItem() ) );
}

void ClassAttPage::slotNewAttribute() {
	//save highlighted item first
	UMLAttribute* selectedAttribute = m_pAttList->at( m_pAttsLB->currentItem() );
	if (selectedAttribute)
		selectedAttribute -> setDoc( m_pDocTE -> text() );

	m_bSigWaiting = true;
	m_pDoc->createUMLObject(m_pClass, Uml::ot_Attribute);
}

#include "classattpage.moc"

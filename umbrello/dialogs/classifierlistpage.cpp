/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classifierlistpage.h"
#include "../classifierlistitem.h"
#include "../umldoc.h"
#include "../class.h"
#include "../enum.h"
#include "../attribute.h"
#include "../operation.h"
#include "../template.h"
#include "../enumliteral.h"
#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>

ClassifierListPage::ClassifierListPage(QWidget* parent, UMLClassifier* classifier,
				       UMLDoc* doc, UMLObject_Type type) : QWidget(parent) {
	itemType = type;
	QString typeName("");
	QString newItemType("");
	if (type == ot_Attribute) {
		typeName = i18n("Attributes");
		newItemType = i18n("N&ew Attribute...");
	} else if (type == ot_Operation) {
		typeName = i18n("Operations");
		newItemType = i18n("N&ew Operation...");
	} else if (type == ot_Template) {
		typeName = i18n("Templates");
		newItemType = i18n("N&ew Template...");
	} else if (type == ot_EnumLiteral) {
		typeName = i18n("Enum Literals");
		newItemType = i18n("N&ew Enum Literal...");
	} else {
		kdWarning() << "unknown listItem type in ClassifierListPage" << endl;
	}

	m_bSigWaiting = false;
	m_pDoc = doc;
	m_pClassifier = classifier;
	m_pMenu = 0;
	int margin = fontMetrics().height();
	setMinimumSize(310,330);

	//main layout contains our two group boxes, the list and the documentation
	QVBoxLayout* mainLayout = new QVBoxLayout( this );
	mainLayout->setSpacing(10);

	//top group box, contains a vertical layout with list box above and buttons below
	m_pItemListGB = new QGroupBox(typeName, this );
	QVBoxLayout* listVBoxLayout = new QVBoxLayout( m_pItemListGB );
	listVBoxLayout->setMargin(margin);
	listVBoxLayout->setSpacing ( 10 );

	//horizontal box contains the list box and the move up/down buttons
	QHBoxLayout* listHBoxLayout = new QHBoxLayout( listVBoxLayout );
	m_pItemListLB = new QListBox(m_pItemListGB);
	listHBoxLayout->addWidget(m_pItemListLB);

	//the move up/down buttons (another vertical box)
	QVBoxLayout* buttonLayout = new QVBoxLayout( listHBoxLayout );
	m_pUpArrowB = new KArrowButton( m_pItemListGB );
	m_pUpArrowB->setEnabled( false );
	buttonLayout->addWidget( m_pUpArrowB );

	m_pDownArrowB = new KArrowButton( m_pItemListGB, Qt::DownArrow );
	m_pDownArrowB->setEnabled( false );
	buttonLayout->addWidget( m_pDownArrowB );

	//the action buttons
	KButtonBox* buttonBox = new KButtonBox(m_pItemListGB);
	buttonBox->addButton( newItemType, this, SLOT(slotNewListItem()) );
	m_pDeleteListItemButton = buttonBox->addButton( i18n("&Delete"),
							  this, SLOT(slotDelete()) );
	m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), this, SLOT(slotProperties()) );
	buttonBox->addButton( i18n("Ne&w Stereotype..."), this, SLOT(slotNewStereotype()) );
	listVBoxLayout->addWidget(buttonBox);

	mainLayout->addWidget(m_pItemListGB);

	m_pDocGB = new QGroupBox(i18n("Documentation"), this);
	QVBoxLayout* docLayout = new QVBoxLayout( m_pDocGB );
	m_pDocTE = new QTextEdit( m_pDocGB );
	docLayout->setMargin(margin);
	docLayout->setSpacing ( 10 );
	docLayout->addWidget( m_pDocTE );
	mainLayout->addWidget(m_pDocGB);

	UMLClassifierListItemList itemList(getItemList());
	
	// add each item in the list to the ListBox and connect each item modified signal
	// to the ListItemModified slot in this class
	for (UMLClassifierListItem* listItem = itemList.first(); listItem != 0; listItem = itemList.next() ) {
		m_pItemListLB->insertItem(listItem->getShortName());
		connect( listItem, SIGNAL(modified()),this,SLOT(slotListItemModified()) );
	}

	enableWidgets(false);//disable widgets until an att is chosen
	m_pOldListItem = 0;
	connect(m_pItemListLB, SIGNAL(clicked(QListBoxItem*)), this, SLOT(slotClicked(QListBoxItem*)));
	connect(m_pItemListLB, SIGNAL(selectionChanged(QListBoxItem*)), this, SLOT(slotClicked(QListBoxItem*)));

	connect(m_pItemListLB, SIGNAL(rightButtonPressed(QListBoxItem*, const QPoint&)),
	        this, SLOT(slotRightButtonPressed(QListBoxItem*, const QPoint&)));

	connect(m_pItemListLB, SIGNAL(rightButtonClicked(QListBoxItem*, const QPoint&)),
	        this, SLOT(slotRightButtonClicked(QListBoxItem*, const QPoint&)));
	connect(m_pDoc, SIGNAL(sigObjectCreated(UMLObject*)), this, SLOT(slotListItemCreated(UMLObject*)));

	connect( m_pUpArrowB, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );
	connect( m_pDownArrowB, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );
	connect( m_pItemListLB, SIGNAL( doubleClicked( QListBoxItem* ) ),
		 this, SLOT( slotDoubleClick( QListBoxItem* ) ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
ClassifierListPage::~ClassifierListPage() {

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassifierListPage::enableWidgets(bool state) {
	m_pDocTE->setEnabled( state );
	//if disabled clear contents
	if( !state ) {
		m_pDocTE->setText( "" );
		m_pUpArrowB->setEnabled( false );
		m_pDownArrowB->setEnabled( false );
		m_pDeleteListItemButton->setEnabled(false);
		m_pPropertiesButton->setEnabled(false);
		return;
	}
	/*now check the order buttons.
		Double check an item is selected
		If only one item in list make sure they are disabled.
		If at top item, only allow down arrow to be enabled.
		If at bottom item, only allow up arrow to be enabled.
	*/
	int index = m_pItemListLB->currentItem();
	if( m_pItemListLB->count() == 1 || index == -1 ) {
		m_pUpArrowB->setEnabled( false );
		m_pDownArrowB->setEnabled( false );
	} else if( index == 0 ) {
		m_pUpArrowB->setEnabled( false );
		m_pDownArrowB->setEnabled( true );
	} else if( index == (int)m_pItemListLB->count() - 1 ) {
		m_pUpArrowB->setEnabled( true );
		m_pDownArrowB->setEnabled( false );
	} else {
		m_pUpArrowB->setEnabled( true );
		m_pDownArrowB->setEnabled( true );
	}
	m_pDeleteListItemButton->setEnabled(true);
	m_pPropertiesButton->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassifierListPage::slotClicked(QListBoxItem*item) {
	//if not first time an item is highlighted
	//save old highlighted item first
	if(m_pOldListItem) {
		m_pOldListItem->setDoc( m_pDocTE->text() );
	}

	//make sure clicked on an item
	if(!item) {
		enableWidgets(false);
		m_pOldListItem = 0;
		m_pItemListLB->clearSelection();
		return;
	}

	UMLClassifierListItem* listItem = getItemList().at( m_pItemListLB->index(item) );

	//now update screen
	m_pDocTE->setText( listItem->getDoc() );
	enableWidgets(true);
	m_pOldListItem = listItem;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassifierListPage::updateObject() {
	saveCurrentItemDocumentation();
	QListBoxItem*i = m_pItemListLB->item(m_pItemListLB->currentItem());
	slotClicked(i);
	
	// The rest of this function does nothing?!
	QStringList stringList;
	int count = m_pItemListLB->count();
	for( int j = 0; j < count ; j++ )
		stringList.append( m_pItemListLB->text( j ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassifierListPage::slotListItemCreated(UMLObject* object) {
	if(!m_bSigWaiting) {
		return;
	}
	if (object->getBaseType() == ot_Datatype)  {
		return;
	}
	int index = m_pItemListLB->count();
	m_pItemListLB ->insertItem((static_cast<UMLClassifierListItem*>(object))->getShortName(), index);
	m_bSigWaiting = false;
	slotClicked(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassifierListPage::slotListItemModified() {
	if(!m_bSigWaiting) {
		return;
	}
	//is this safe???
	UMLClassifierListItem* object = const_cast<UMLClassifierListItem*>(dynamic_cast<const UMLClassifierListItem*>(sender()));
	int index = m_pItemListLB->currentItem();
	m_pItemListLB->changeItem(object->getShortName(), index);
	m_bSigWaiting = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassifierListPage::slotRightButtonClicked(QListBoxItem* /*item*/, const QPoint& /* p*/) {
	if (m_pMenu) {
		m_pMenu->hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassifierListPage::slotRightButtonPressed(QListBoxItem* item, const QPoint& p) {
	ListPopupMenu::Menu_Type type = ListPopupMenu::mt_Undefined;
	if (item) { //pressed on a list item
		if (itemType == ot_Attribute) {
			type = ListPopupMenu::mt_Attribute_Selected;
		} else if (itemType == ot_Operation) {
			type = ListPopupMenu::mt_Operation_Selected;
		} else if (itemType == ot_Template) {
			type = ListPopupMenu::mt_Template_Selected;
		} else if (itemType == ot_EnumLiteral) {
			type = ListPopupMenu::mt_EnumLiteral_Selected;
		} else {
			kdWarning() << "unknown type in ClassifierListPage" << endl;
		}
	} else { //pressed into fresh air
		if (itemType == ot_Attribute) {
			type = ListPopupMenu::mt_New_Attribute;
		} else if (itemType == ot_Operation) {
			type = ListPopupMenu::mt_New_Operation;
		} else if (itemType == ot_Template) {
			type = ListPopupMenu::mt_New_Template;
		} else if (itemType == ot_EnumLiteral) {
			type = ListPopupMenu::mt_New_EnumLiteral;
		} else {
			kdWarning() << "unknown type in ClassifierListPage" << endl;
		}
	}
	if(m_pMenu) {
		m_pMenu->hide();
		disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
		delete m_pMenu;
		m_pMenu = 0;
	}
	m_pMenu = new ListPopupMenu(this, type);

	m_pMenu->popup(p);
	connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void ClassifierListPage::slotPopupMenuSel(int id) {
	UMLClassifierListItem* listItem = getItemList().at( m_pItemListLB->currentItem() );
	if(!listItem && id != ListPopupMenu::mt_New_Attribute) {
		kdDebug() << "can't find att from selection" << endl;
		return;
	}
	switch(id) {
		case ListPopupMenu::mt_New_Attribute:
		case ListPopupMenu::mt_New_Operation:
		case ListPopupMenu::mt_New_Template:
		case ListPopupMenu::mt_New_EnumLiteral:
			slotNewListItem();
			break;

		case ListPopupMenu::mt_Delete:
			slotDelete();
			break;

		case ListPopupMenu::mt_Rename:
			m_bSigWaiting = true;
			m_pDoc->renameChildUMLObject(listItem);
			break;

		case ListPopupMenu::mt_Properties:
			slotProperties();
			break;
	}
}

void ClassifierListPage::slotUpClicked() {
	int count = m_pItemListLB->count();
	int index = m_pItemListLB->currentItem();
	//shouldn't occur, but just in case
	if( count <= 1 || index <= 0 )
		return;

	//swap the text around in the ListBox
	QString aboveString = m_pItemListLB->text( index - 1 );
	QString currentString = m_pItemListLB->text( index );
	m_pItemListLB->changeItem( currentString, index -1 );
	m_pItemListLB->changeItem( aboveString, index );
	//set the moved item selected
	QListBoxItem* item = m_pItemListLB->item( index - 1 );
	m_pItemListLB->setSelected( item, true );
	
	//now change around in the list
	UMLClassifierListItem* currentAtt = getItemList().at( index );
	takeClassifier(currentAtt);
	addClassifier(currentAtt, index - 1);
	slotClicked( item );
}

void ClassifierListPage::slotDownClicked() {
	int count = m_pItemListLB->count();
	int index = m_pItemListLB->currentItem();
	//shouldn't occur, but just in case
	if( count <= 1 || index >= count - 1 )
		return;

	//swap the text around in the ListBox
	QString belowString = m_pItemListLB->text( index + 1 );
	QString currentString = m_pItemListLB->text( index );
	m_pItemListLB->changeItem( currentString, index + 1 );
	m_pItemListLB->changeItem( belowString, index );
	//set the moved item selected
	QListBoxItem* item = m_pItemListLB->item( index + 1 );
	m_pItemListLB->setSelected( item, true );
	//now change around in the list
	UMLClassifierListItem* currentAtt = getItemList().at( index );
	takeClassifier(currentAtt);
	addClassifier(currentAtt, index + 2);
	slotClicked( item );
}

void ClassifierListPage::slotDoubleClick( QListBoxItem* item ) {
	if( !item )
		return;

	UMLClassifierListItem* listItem  = getItemList().at( m_pItemListLB->index( item ) );
	if( !listItem ) {
		kdDebug() << "can't find att from selection" << endl;
		return;
	}

	if( listItem->showPropertiesDialogue(this) ) {
		m_pItemListLB->changeItem( listItem->getShortName(), m_pItemListLB->index(item) );
	}
}

void ClassifierListPage::slotDelete() {
	UMLClassifierListItem* selectedItem = getItemList().at( m_pItemListLB->currentItem() );
	//should really wait for signal back
	//but really shouldn't matter
	m_pDoc->removeUMLObject(selectedItem);
	m_pItemListLB->removeItem( m_pItemListLB->currentItem());
	m_pOldListItem = 0;
	slotClicked(0);
}

void ClassifierListPage::slotProperties() {
	saveCurrentItemDocumentation();
	slotDoubleClick( m_pItemListLB->item( m_pItemListLB->currentItem() ) );
}

void ClassifierListPage::slotNewListItem() {
	saveCurrentItemDocumentation();
	m_bSigWaiting = true;
	m_pDoc->createChildObject(m_pClassifier, itemType);
}

void ClassifierListPage::slotNewStereotype() {
	saveCurrentItemDocumentation();
	m_bSigWaiting = true;
	m_pDoc->createStereotype(m_pClassifier, itemType);
}

void ClassifierListPage::saveCurrentItemDocumentation() {
	UMLClassifierListItem* selectedItem = getItemList().at( m_pItemListLB->currentItem() );
	if (selectedItem) {
		selectedItem->setDoc( m_pDocTE->text() );
	}
}

UMLClassifierListItemList ClassifierListPage::getItemList() {
	switch (itemType) {
		case ot_Attribute: {
			UMLClass* classifier = dynamic_cast<UMLClass*>(m_pClassifier);
			if (classifier) {
				return UMLClassifierListItemList(*classifier->getAttList());
			}
			break;
		}
		case ot_Operation: {
			return m_pClassifier->getOpList();
		}
		case ot_Template: {
			UMLClass* classifier = dynamic_cast<UMLClass*>(m_pClassifier);
			if (classifier) {
				return UMLClassifierListItemList(*classifier->getTemplateList());
			}
			break;
		}
		case ot_EnumLiteral: {
			UMLEnum* classifier = dynamic_cast<UMLEnum*>(m_pClassifier);
			if (classifier) {
				return UMLClassifierListItemList(*classifier->getEnumLiteralList());
			}
			break;
		}
		default: {
			kdWarning() << "unknown type in ClassifierListPage" << endl;
			return UMLClassifierListItemList();
		}
	}
	kdError() << "ClassifierListPage is in an inconsistent state!" << endl;
	return UMLClassifierListItemList();
}
	
bool ClassifierListPage::addClassifier(UMLClassifierListItem* classifier, int position) {
	switch (itemType) {
		case ot_Attribute: {
			UMLClass* c = dynamic_cast<UMLClass*>(m_pClassifier);
			if (c) {
				return c->addAttribute(dynamic_cast<UMLAttribute*>(classifier), 0, position);
			}
			break;
		}
		case ot_Operation: {
			return m_pClassifier->addOperation(dynamic_cast<UMLOperation*>(classifier), position);
		case ot_Template:
			UMLClass* c = dynamic_cast<UMLClass*>(m_pClassifier);
			if (c) {
				return c->addTemplate(dynamic_cast<UMLTemplate*>(classifier), position);
			}
			break;
		}
		case ot_EnumLiteral: {
			UMLEnum* c = dynamic_cast<UMLEnum*>(m_pClassifier);
			if (c) {
				return c->addEnumLiteral(dynamic_cast<UMLEnumLiteral*>(classifier), position);
			}
			break;
		}
		default: {
			kdWarning() << "unknown type in ClassifierListPage" << endl;
			return false;
		}
	}
	kdError() << "ClassifierListPage::addClassifier unable to handle classifier type in current state" << endl;
	return false;
}

UMLClassifierListItem* ClassifierListPage::takeClassifier(UMLClassifierListItem* classifier) {
	switch (itemType) {
		case ot_Attribute: {
			UMLClass* c = dynamic_cast<UMLClass*>(m_pClassifier);
			if (c) {
				return c->takeAttribute(dynamic_cast<UMLAttribute*>(classifier));
			}
			break;
		}
		case ot_Operation: {
			return m_pClassifier->takeOperation(dynamic_cast<UMLOperation*>(classifier));
		}
		case ot_Template: {
			UMLClass* c = dynamic_cast<UMLClass*>(m_pClassifier);
			if (c) {
				return c->takeTemplate(dynamic_cast<UMLTemplate*>(classifier));
			}
			break;
		}
		case ot_EnumLiteral: {
			UMLEnum* c = dynamic_cast<UMLEnum*>(m_pClassifier);
			if (c) {
				return c->takeEnumLiteral(dynamic_cast<UMLEnumLiteral*>(classifier));
			}
			break;
		}
		default: {
			kdWarning() << "unknown type in ClassifierListPage" << endl;
			return 0;
		}
	}
	kdError() << "ClassifierListPage::takeClassifier unable to handle classifier type in current state" << endl;
	return 0;
}

int ClassifierListPage::removeClassifier(UMLClassifierListItem* classifier) {
	switch (itemType) {
		case ot_Attribute: {
			UMLClass* c = dynamic_cast<UMLClass*>(m_pClassifier);
			if (c) {
				return c->removeAttribute(dynamic_cast<UMLAttribute*>(classifier));
			}
			break;
		}
		case ot_Operation: {
			return m_pClassifier->removeOperation(dynamic_cast<UMLOperation*>(classifier));
		}
		case ot_Template: {
			UMLClass* c = dynamic_cast<UMLClass*>(m_pClassifier);
			if (c) {
				return c->removeTemplate(dynamic_cast<UMLTemplate*>(classifier));
			}
			break;
		}
		case ot_EnumLiteral: {
			UMLEnum* c = dynamic_cast<UMLEnum*>(m_pClassifier);
			if (c) {
				return c->removeEnumLiteral(dynamic_cast<UMLEnumLiteral*>(classifier));
			}
			break;
		}
		default: {
			kdWarning() << "unknown type in ClassifierListPage" << endl;
			return -1;
		}
	}
	kdError() << "ClassifierListPage::removeClassifier unable to handle classifier type in current state" << endl;
	return -1;
}
#include "classifierlistpage.moc"

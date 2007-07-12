/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "classifierlistpage.h"
#include "../classifierlistitem.h"
#include "../umldoc.h"
#include "../classifier.h"
#include "../enum.h"
#include "../entity.h"
#include "../attribute.h"
#include "../operation.h"
#include "../template.h"
#include "../enumliteral.h"
#include "../entityattribute.h"
#include "../object_factory.h"
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>

using namespace Uml;

ClassifierListPage::ClassifierListPage(QWidget* parent, UMLClassifier* classifier,
                                       UMLDoc* doc, Uml::Object_Type type) : QWidget(parent) {
    m_itemType = type;
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
    } else if (type == ot_EntityAttribute) {
        typeName = i18n("Entity Attributes");
        newItemType = i18n("N&ew Entity Attribute...");
    } else {
        kWarning() << "unknown listItem type in ClassifierListPage" << endl;
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
    m_pTopArrowB = new KArrowButton( m_pItemListGB );
    m_pTopArrowB->setEnabled( false );
    buttonLayout->addWidget( m_pTopArrowB );

    m_pUpArrowB = new KArrowButton( m_pItemListGB );
    m_pUpArrowB->setEnabled( false );
    buttonLayout->addWidget( m_pUpArrowB );

    m_pDownArrowB = new KArrowButton( m_pItemListGB, Qt::DownArrow );
    m_pDownArrowB->setEnabled( false );
    buttonLayout->addWidget( m_pDownArrowB );

    m_pBottomArrowB = new KArrowButton( m_pItemListGB, Qt::DownArrow );
    m_pBottomArrowB->setEnabled( false );
    buttonLayout->addWidget( m_pBottomArrowB );

    //the action buttons
    KButtonBox* buttonBox = new KButtonBox(m_pItemListGB);
    buttonBox->addButton( newItemType, this, SLOT(slotNewListItem()) );
    m_pDeleteListItemButton = buttonBox->addButton( i18n("&Delete"),
                              this, SLOT(slotDelete()) );
    m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), this, SLOT(slotProperties()) );
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
        m_pItemListLB->insertItem(listItem->toString(Uml::st_SigNoVis));
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

    connect( m_pTopArrowB, SIGNAL( clicked() ), this, SLOT( slotTopClicked() ) );
    connect( m_pUpArrowB, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );
    connect( m_pDownArrowB, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );
    connect( m_pBottomArrowB, SIGNAL( clicked() ), this, SLOT( slotBottomClicked() ) );
    connect( m_pItemListLB, SIGNAL( doubleClicked( QListBoxItem* ) ),
             this, SLOT( slotDoubleClick( QListBoxItem* ) ) );
}

ClassifierListPage::~ClassifierListPage() {

}

void ClassifierListPage::enableWidgets(bool state) {
    m_pDocTE->setEnabled( state );
    //if disabled clear contents
    if( !state ) {
        m_pDocTE->setText( "" );
        m_pTopArrowB->setEnabled( false );
        m_pUpArrowB->setEnabled( false );
        m_pDownArrowB->setEnabled( false );
        m_pBottomArrowB->setEnabled( false );
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
        m_pTopArrowB->setEnabled( false );
        m_pUpArrowB->setEnabled( false );
        m_pDownArrowB->setEnabled( false );
        m_pBottomArrowB->setEnabled( false );
    } else if( index == 0 ) {
        m_pTopArrowB->setEnabled( false );
        m_pUpArrowB->setEnabled( false );
        m_pDownArrowB->setEnabled( true );
        m_pBottomArrowB->setEnabled( true );
    } else if( index == (int)m_pItemListLB->count() - 1 ) {
        m_pTopArrowB->setEnabled( true );
        m_pUpArrowB->setEnabled( true );
        m_pDownArrowB->setEnabled( false );
        m_pBottomArrowB->setEnabled( false );
    } else {
        m_pTopArrowB->setEnabled( true );
        m_pUpArrowB->setEnabled( true );
        m_pDownArrowB->setEnabled( true );
        m_pBottomArrowB->setEnabled( true );
    }
    m_pDeleteListItemButton->setEnabled(true);
    m_pPropertiesButton->setEnabled(true);
}

void ClassifierListPage::slotClicked(QListBoxItem*item) {
    //if not first time an item is highlighted
    //save old highlighted item first
    if(m_pOldListItem) {
        m_pOldListItem->setDoc( m_pDocTE->text() );
    }

    // make sure clicked on an item
    // it is impossible to deselect all items, because our list box has keyboard
    // focus and so at least one item is always selected; this doesn't happen, if
    // there are no items of course;
    //
    // for more information see Qt doc for void QListBox::clearSelection()
    UMLClassifierListItem* listItem;
    if (item == NULL) {
        if (m_pItemListLB->count() == 0) {
            enableWidgets(false);
            m_pOldListItem = 0;
            m_pItemListLB->clearSelection();
            return;
        }
        m_pItemListLB->setSelected(0, true);
        listItem = getItemList().at(0);
    } else {
        listItem = getItemList().at( m_pItemListLB->index(item) );
    }

    if (listItem) {
        //now update screen
        m_pDocTE->setText( listItem->getDoc() );
        enableWidgets(true);
        m_pOldListItem = listItem;
    }
}

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

void ClassifierListPage::slotListItemCreated(UMLObject* object) {
    if(!m_bSigWaiting) {
        return;
    }
    UMLClassifierListItem *listItem = dynamic_cast<UMLClassifierListItem*>(object);
    if (listItem == NULL)  {
        return;
    }
    int index = m_pItemListLB->count();
    m_pItemListLB->insertItem(listItem->toString(Uml::st_SigNoVis), index);
    m_bSigWaiting = false;

    // now select the new item, so that the user can go on adding doc or calling
    // the property dialog
    m_pItemListLB->setSelected(index, true);
    slotClicked(m_pItemListLB->item(index));
}

void ClassifierListPage::slotListItemModified() {
    if(!m_bSigWaiting) {
        return;
    }
    //is this safe???
    UMLClassifierListItem* object = const_cast<UMLClassifierListItem*>(dynamic_cast<const UMLClassifierListItem*>(sender()));
    if (object == NULL)
        return;
    int index = m_pItemListLB->currentItem();
    m_pItemListLB->changeItem(object->toString(Uml::st_SigNoVis), index);
    m_bSigWaiting = false;
}

void ClassifierListPage::slotRightButtonClicked(QListBoxItem* /*item*/, const QPoint& /* p*/) {
    if (m_pMenu) {
        m_pMenu->hide();
        disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(slotPopupMenuSel(int)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void ClassifierListPage::slotRightButtonPressed(QListBoxItem* item, const QPoint& p) {
    ListPopupMenu::Menu_Type type = ListPopupMenu::mt_Undefined;
    if (item) { //pressed on a list item
        if (m_itemType == ot_Attribute) {
            type = ListPopupMenu::mt_Attribute_Selected;
        } else if (m_itemType == ot_Operation) {
            type = ListPopupMenu::mt_Operation_Selected;
        } else if (m_itemType == ot_Template) {
            type = ListPopupMenu::mt_Template_Selected;
        } else if (m_itemType == ot_EnumLiteral) {
            type = ListPopupMenu::mt_EnumLiteral_Selected;
        } else if (m_itemType == ot_EntityAttribute) {
            type = ListPopupMenu::mt_EntityAttribute_Selected;
        } else {
            kWarning() << "unknown type in ClassifierListPage" << endl;
        }
    } else { //pressed into fresh air
        if (m_itemType == ot_Attribute) {
            type = ListPopupMenu::mt_New_Attribute;
        } else if (m_itemType == ot_Operation) {
            type = ListPopupMenu::mt_New_Operation;
        } else if (m_itemType == ot_Template) {
            type = ListPopupMenu::mt_New_Template;
        } else if (m_itemType == ot_EnumLiteral) {
            type = ListPopupMenu::mt_New_EnumLiteral;
        } else if (m_itemType == ot_EntityAttribute) {
            type = ListPopupMenu::mt_New_EntityAttribute;
        } else {
            kWarning() << "unknown type in ClassifierListPage" << endl;
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

void ClassifierListPage::slotPopupMenuSel(int id) {
    UMLClassifierListItem* listItem = getItemList().at( m_pItemListLB->currentItem() );
    if(!listItem && id != ListPopupMenu::mt_New_Attribute) {
        kDebug() << "can't find att from selection" << endl;
        return;
    }
    switch(id) {
    case ListPopupMenu::mt_New_Attribute:
    case ListPopupMenu::mt_New_Operation:
    case ListPopupMenu::mt_New_Template:
    case ListPopupMenu::mt_New_EnumLiteral:
    case ListPopupMenu::mt_New_EntityAttribute:
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

void ClassifierListPage::printItemList(QString prologue) {
#ifdef VERBOSE_DEBUGGING
    UMLClassifierListItem* item;
    QString buf;
    UMLClassifierListItemList itemList = getItemList();
    for (UMLClassifierListItemListIt it(itemList); (item = it.current()) != NULL; ++it)
        buf.append(' ' + item->getName());
    kDebug() << prologue << buf << endl;
#endif
}

void ClassifierListPage::slotTopClicked() {
    int count = m_pItemListLB->count();
    int index = m_pItemListLB->currentItem();
    //shouldn't occur, but just in case
    if( count <= 1 || index <= 0 )
        return;
    m_pOldListItem = NULL;

    //swap the text around in the ListBox
    QString currentString = m_pItemListLB->text( index );
    m_pItemListLB->removeItem( index );
    m_pItemListLB->insertItem( currentString, 0 );
    //set the moved item selected
    QListBoxItem* item = m_pItemListLB->item( 0 );
    m_pItemListLB->setSelected( item, true );

    //now change around in the list
    printItemList("itemList before change: ");
    UMLClassifierListItem* currentAtt = getItemList().at(index);
    // NB: The index in the m_pItemListLB is not necessarily the same
    //     as the index in the UMLClassifier::m_List.
    //     Reason: getItemList() returns only a subset of all entries
    //     in UMLClassifier::m_List.
    takeItem(currentAtt, true, index);  // now we index the UMLClassifier::m_List
    kDebug() << "ClassifierListPage::slotTopClicked(" << currentAtt->getName()
            << "): peer index in UMLCanvasItem::m_List is " << index << endl;
    addClassifier(currentAtt, 0);
    printItemList("itemList after change: ");
    slotClicked(item);
}

void ClassifierListPage::slotUpClicked() {
    int count = m_pItemListLB->count();
    int index = m_pItemListLB->currentItem();
    //shouldn't occur, but just in case
    if( count <= 1 || index <= 0 )
        return;
    m_pOldListItem = NULL;

    //swap the text around in the ListBox
    QString aboveString = m_pItemListLB->text( index - 1 );
    QString currentString = m_pItemListLB->text( index );
    m_pItemListLB->changeItem( currentString, index -1 );
    m_pItemListLB->changeItem( aboveString, index );
    //set the moved item selected
    QListBoxItem* item = m_pItemListLB->item( index - 1 );
    m_pItemListLB->setSelected( item, true );

    //now change around in the list
    printItemList("itemList before change: ");
    UMLClassifierListItem* currentAtt = getItemList().at(index);
    // NB: The index in the m_pItemListLB is not necessarily the same
    //     as the index in the UMLClassifier::m_List.
    //     Reason: getItemList() returns only a subset of all entries
    //     in UMLClassifier::m_List.
    takeItem(currentAtt, true, index);  // now we index the UMLClassifier::m_List
    kDebug() << "ClassifierListPage::slotUpClicked(" << currentAtt->getName()
        << "): peer index in UMLCanvasItem::m_List is " << index << endl;
    if (index == -1)
        index = 0;
    addClassifier(currentAtt, index);
    printItemList("itemList after change: ");
    slotClicked( item );
}

void ClassifierListPage::slotDownClicked() {
    int count = m_pItemListLB->count();
    int index = m_pItemListLB->currentItem();
    //shouldn't occur, but just in case
    if( count <= 1 || index >= count - 1 )
        return;
    m_pOldListItem = NULL;

    //swap the text around in the ListBox
    QString belowString = m_pItemListLB->text( index + 1 );
    QString currentString = m_pItemListLB->text( index );
    m_pItemListLB->changeItem( currentString, index + 1 );
    m_pItemListLB->changeItem( belowString, index );
    //set the moved item selected
    QListBoxItem* item = m_pItemListLB->item( index + 1 );
    m_pItemListLB->setSelected( item, true );
    //now change around in the list
    printItemList("itemList before change: ");
    UMLClassifierListItem* currentAtt = getItemList().at(index);
    // NB: The index in the m_pItemListLB is not necessarily the same
    //     as the index in the UMLClassifier::m_List.
    //     Reason: getItemList() returns only a subset of all entries
    //     in UMLClassifier::m_List.
    takeItem(currentAtt, false, index);  // now we index the UMLClassifier::m_List
    kDebug() << "ClassifierListPage::slotDownClicked(" << currentAtt->getName()
        << "): peer index in UMLCanvasItem::m_List is " << index << endl;
    if (index != -1)
        index++;   // because we want to go _after_ the following peer item
    addClassifier(currentAtt, index);
    printItemList("itemList after change: ");
    slotClicked( item );
}

void ClassifierListPage::slotBottomClicked() {
    int count = m_pItemListLB->count();
    int index = m_pItemListLB->currentItem();
    //shouldn't occur, but just in case
    if( count <= 1 || index >= count - 1 )
        return;
    m_pOldListItem = NULL;

    //swap the text around in the ListBox
    QString currentString = m_pItemListLB->text( index );
    m_pItemListLB->removeItem( index );
    m_pItemListLB->insertItem( currentString, m_pItemListLB->count() );
    //set the moved item selected
    QListBoxItem* item = m_pItemListLB->item( m_pItemListLB->count() - 1 );
    m_pItemListLB->setSelected( item, true );

    //now change around in the list
    printItemList("itemList before change: ");
    UMLClassifierListItem* currentAtt = getItemList().at(index);
    // NB: The index in the m_pItemListLB is not necessarily the same
    //     as the index in the UMLClassifier::m_List.
    //     Reason: getItemList() returns only a subset of all entries
    //     in UMLClassifier::m_List.
    takeItem(currentAtt, false, index);  // now we index the UMLClassifier::m_List
    kDebug() << "ClassifierListPage::slotDownClicked(" << currentAtt->getName()
            << "): peer index in UMLCanvasItem::m_List is " << index << endl;
    addClassifier(currentAtt, getItemList().count());
    printItemList("itemList after change: ");
    slotClicked( item );
}

void ClassifierListPage::slotDoubleClick( QListBoxItem* item ) {
    if( !item )
        return;

    UMLClassifierListItem* listItem  = getItemList().at( m_pItemListLB->index( item ) );
    if( !listItem ) {
        kDebug() << "can't find att from selection" << endl;
        return;
    }

    if( listItem->showPropertiesDialog(this) ) {
        m_pItemListLB->changeItem( listItem->toString(Uml::st_SigNoVis), m_pItemListLB->index(item) );
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
    Object_Factory::createChildObject(m_pClassifier, m_itemType);
}

void ClassifierListPage::saveCurrentItemDocumentation() {
    UMLClassifierListItem* selectedItem = getItemList().at( m_pItemListLB->currentItem() );
    if (selectedItem) {
        selectedItem->setDoc( m_pDocTE->text() );
    }
}

UMLClassifierListItemList ClassifierListPage::getItemList() {
    return m_pClassifier->getFilteredList(m_itemType);
}

bool ClassifierListPage::addClassifier(UMLClassifierListItem* listitem, int position) {
    switch (m_itemType) {
    case ot_Attribute: {
            UMLAttribute *att = dynamic_cast<UMLAttribute*>(listitem);
            return m_pClassifier->addAttribute(att, NULL, position);
        }
    case ot_Operation: {
            UMLOperation *op = dynamic_cast<UMLOperation*>(listitem);
            return m_pClassifier->addOperation(op, position);
        }
    case ot_Template: {
            UMLTemplate* t = dynamic_cast<UMLTemplate*>(listitem);
            return m_pClassifier->addTemplate(t, position);
        }
    case ot_EnumLiteral: {
            UMLEnum* c = dynamic_cast<UMLEnum*>(m_pClassifier);
            if (c) {
                return c->addEnumLiteral(dynamic_cast<UMLEnumLiteral*>(listitem), position);
            }
            break;
        }
    case ot_EntityAttribute: {
            UMLEntity* c = dynamic_cast<UMLEntity*>(m_pClassifier);
            if (c) {
                return c->addEntityAttribute(dynamic_cast<UMLEntityAttribute*>(listitem), position);
            }
            break;
        }
    default: {
            kWarning() << "unknown type in ClassifierListPage" << endl;
            return false;
        }
    }
    kError() << "ClassifierListPage::addClassifier unable to handle listitem type in current state" << endl;
    return false;
}

bool ClassifierListPage::takeItem(UMLClassifierListItem* listItem,
                                  bool seekPeerBefore, int &peerIndex) {
    int wasAtIndex = m_pClassifier->takeItem(listItem);
    if (wasAtIndex == -1)
        return false;
    kapp->processEvents();
    peerIndex = -1;
    UMLObject *o;
    const Uml::Object_Type seekType = listItem->getBaseType();
    UMLObjectList listItems = m_pClassifier->subordinates();
    UMLObjectListIt it(listItems);
    for (int i = 0; (o = it.current()) != NULL; ++i, ++it) {
        if (seekPeerBefore) {
            if (i >= wasAtIndex)
                break;
            if (o->getBaseType() == seekType)
                peerIndex = i;
        } else {    // seekPeerAfter
            if (i < wasAtIndex)
                continue;
            if (o->getBaseType() == seekType) {
                peerIndex = i;
                break;
            }
        }
    }
    return true;
}


#include "classifierlistpage.moc"


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "classifierlistpage.h"

#include <kdebug.h>
#include <kdialogbuttonbox.h>
#include <klocale.h>
#include <QApplication>

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


using namespace Uml;

ClassifierListPage::ClassifierListPage(QWidget* parent, UMLClassifier* classifier,
                                       UMLDoc* doc, Uml::Object_Type type) : QWidget(parent)
{
    m_itemType = type;
    m_bSigWaiting = false;
    m_pDoc = doc;
    m_pClassifier = classifier;
    m_pMenu = 0;

    setupPage();
}

void ClassifierListPage::setupPage()
{
    int margin = fontMetrics().height();
    setMinimumSize(310,330);

    //main layout contains our two group boxes, the list and the documentation
    QVBoxLayout* mainLayout = new QVBoxLayout( this );
    mainLayout->setSpacing(10);

    setupListGroup(margin);
    mainLayout->addWidget(m_pItemListGB);

    setupDocumentationGroup(margin);
    mainLayout->addWidget(m_pDocGB);

    reloadItemListBox();

    enableWidgets(false);//disable widgets until an att is chosen
    m_pOldListItem = 0;

    connect(m_pItemListLB, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotClicked(QListWidgetItem*)));
    connect(m_pItemListLB, SIGNAL(itemDoubleClicked( QListWidgetItem*)), this, SLOT(slotDoubleClick(QListWidgetItem*)));
    connect(m_pItemListLB, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotRightButtonPressed(const QPoint&)));

    connect(m_pDoc, SIGNAL(sigObjectCreated(UMLObject*)), this, SLOT(slotListItemCreated(UMLObject*)));

    connect(m_pTopArrowB, SIGNAL( clicked() ), this, SLOT( slotTopClicked() ) );
    connect(m_pUpArrowB, SIGNAL( clicked() ), this, SLOT( slotUpClicked() ) );
    connect(m_pDownArrowB, SIGNAL( clicked() ), this, SLOT( slotDownClicked() ) );
    connect(m_pBottomArrowB, SIGNAL( clicked() ), this, SLOT( slotBottomClicked() ) );
}

void ClassifierListPage::setupListGroup(int margin)
{
    QString typeName;
    QString newItemType;

    switch (m_itemType) {
    case ot_Attribute:
        typeName = i18n("Attributes");
        newItemType = i18n("N&ew Attribute...");
        break;
    case ot_Operation:
        typeName = i18n("Operations");
        newItemType = i18n("N&ew Operation...");
        break;
    case ot_Template:
        typeName = i18n("Templates");
        newItemType = i18n("N&ew Template...");
        break;
    case ot_EnumLiteral:
        typeName = i18n("Enum Literals");
        newItemType = i18n("N&ew Enum Literal...");
        break;
    case ot_EntityAttribute:
        typeName = i18n("Entity Attributes");
        newItemType = i18n("N&ew Entity Attribute...");
        break;
    case ot_EntityConstraint:
        typeName = i18n( "Constraints" );
        newItemType = i18n( "N&ew Constraint..." );
        break;
    default:
        uWarning() << "unknown listItem type in ClassifierListPage";
        break;
    }

    //top group box, contains a vertical layout with list box above and buttons below
    m_pItemListGB = new QGroupBox(typeName, this );
    QVBoxLayout* listVBoxLayout = new QVBoxLayout( m_pItemListGB );
    listVBoxLayout->setMargin(margin);
    listVBoxLayout->setSpacing( 10 );

    //horizontal box contains the list box and the move up/down buttons
    QHBoxLayout* listHBoxLayout = new QHBoxLayout();
    listHBoxLayout->setSpacing(10);
    listVBoxLayout->addItem( listHBoxLayout );
    m_pItemListLB = new QListWidget(m_pItemListGB);
    m_pItemListLB->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pItemListLB->setContextMenuPolicy(Qt::CustomContextMenu);
    listHBoxLayout->addWidget(m_pItemListLB);

    setupMoveButtons(listHBoxLayout);
    setupActionButtons(newItemType, listVBoxLayout);
}

void ClassifierListPage::setupMoveButtons(QHBoxLayout* parentLayout)
{
    QVBoxLayout* buttonLayout = new QVBoxLayout();
    parentLayout->addItem( buttonLayout );

    m_pTopArrowB = new KArrowButton( m_pItemListGB );
    m_pTopArrowB->setEnabled( false );
    m_pTopArrowB->setToolTip(i18n("Move selected item to the top"));
    buttonLayout->addWidget( m_pTopArrowB );

    m_pUpArrowB = new KArrowButton( m_pItemListGB );
    m_pUpArrowB->setEnabled( false );
    m_pUpArrowB->setToolTip(i18n("Move selected item up"));
    buttonLayout->addWidget( m_pUpArrowB );

    m_pDownArrowB = new KArrowButton( m_pItemListGB, Qt::DownArrow );
    m_pDownArrowB->setEnabled( false );
    m_pDownArrowB->setToolTip(i18n("Move selected item down"));
    buttonLayout->addWidget( m_pDownArrowB );

    m_pBottomArrowB = new KArrowButton( m_pItemListGB, Qt::DownArrow );
    m_pBottomArrowB->setEnabled( false );
    m_pBottomArrowB->setToolTip(i18n("Move selected item to the bottom"));
    buttonLayout->addWidget( m_pBottomArrowB );
}

void ClassifierListPage::setupActionButtons(const QString& itemType, QVBoxLayout* parentLayout)
{
    KDialogButtonBox* buttonBox = new KDialogButtonBox(m_pItemListGB);
    m_pNewClassifierListItemButton = buttonBox->addButton( itemType, KDialogButtonBox::ActionRole, this,
                          SLOT(slotNewListItem()) );
    m_pDeleteListItemButton = buttonBox->addButton( i18n("&Delete"),
                              KDialogButtonBox::ActionRole, this, SLOT(slotDelete()) );
    m_pPropertiesButton = buttonBox->addButton( i18n("&Properties"), KDialogButtonBox::ActionRole, this,SLOT(slotProperties()) );
    parentLayout->addWidget(buttonBox);
}

void ClassifierListPage::setupDocumentationGroup(int margin)
{
    m_pDocGB = new QGroupBox(i18n("Documentation"), this);
    QVBoxLayout* docLayout = new QVBoxLayout(m_pDocGB);
    docLayout->setSpacing(10);
    docLayout->setMargin(margin);
    if (m_itemType == ot_Operation) {
        m_pDocTE = new QTextEdit();
        m_pCodeTE = new QTextEdit();
        QTabWidget* tabWidget = new QTabWidget();
        tabWidget->addTab(m_pDocTE, i18n("Comment"));
        tabWidget->addTab(m_pCodeTE, i18n("Source Code"));
        docLayout->addWidget(tabWidget);
    }
    else {
        m_pDocTE = new QTextEdit();
        docLayout->addWidget(m_pDocTE);
    }
}

void ClassifierListPage::reloadItemListBox()
{
    UMLClassifierListItemList itemList(getItemList());

    // remove any items if present
    m_pItemListLB->clear();

    // add each item in the list to the ListBox and connect each item modified signal
    // to the ListItemModified slot in this class
    foreach (UMLClassifierListItem* listItem, itemList ) {
        m_pItemListLB->addItem(listItem->toString(Uml::st_SigNoVis));
        connect( listItem, SIGNAL(modified()),this,SLOT(slotListItemModified()) );
    }
}

ClassifierListPage::~ClassifierListPage()
{
}

void ClassifierListPage::enableWidgets(bool state)
{
    m_pDocTE->setEnabled( state );
    if (m_itemType == ot_Operation) {
        m_pCodeTE->setEnabled( state );
    }
    //if disabled clear contents
    if ( !state ) {
        m_pDocTE->setText( "" );
        if (m_itemType == ot_Operation) {
            m_pCodeTE->setText( "" );
        }
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
    int index = m_pItemListLB->currentRow();
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
    } else if( index == m_pItemListLB->count() - 1 ) {
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

void ClassifierListPage::slotClicked(QListWidgetItem* item)
{
    //if not first time an item is highlighted
    //save old highlighted item first
    if (m_pOldListItem) {
        m_pOldListItem->setDoc( m_pDocTE->toPlainText() );
        if (m_itemType == ot_Operation) {
            UMLOperation* op = dynamic_cast<UMLOperation*>(m_pOldListItem);
            op->setSourceCode( m_pCodeTE->toPlainText() );
        }
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
        m_pItemListLB->setCurrentRow(0);
        listItem = getItemList().at(0);
    } else {
        int relativeItemIndex = m_pItemListLB->row(item);
        if (relativeItemIndex < 0) {
            uDebug() << "Cannot find item in list.";
        }
        else {
            listItem = getItemList().at( relativeItemIndex );
        }
    }

    if (listItem) {
        //now update screen
        m_pDocTE->setText( listItem->getDoc() );
        if (m_itemType == ot_Operation) {
            m_pCodeTE->setText( dynamic_cast<UMLOperation*>(listItem)->getSourceCode() );
        }
        enableWidgets(true);
        m_pOldListItem = listItem;
    }
}

void ClassifierListPage::updateObject()
{
    saveCurrentItemDocumentation();
    QListWidgetItem* i = m_pItemListLB->currentItem();
    slotClicked(i);
}

void ClassifierListPage::slotListItemCreated(UMLObject* object)
{
    if (!m_bSigWaiting) {
        return;
    }
    UMLClassifierListItem *listItem = dynamic_cast<UMLClassifierListItem*>(object);
    if (listItem == NULL)  {
        return;
    }

    int index = calculateNewIndex(listItem->getBaseType());

    m_pItemListLB->insertItem(index, listItem->toString(Uml::st_SigNoVis));
    m_bSigWaiting = false;

    // now select the new item, so that the user can go on adding doc or calling
    // the property dialog
    m_pItemListLB->setCurrentRow(index);
    slotClicked(m_pItemListLB->item(index));
    connect( object, SIGNAL( modified() ), this, SLOT( slotListItemModified() ) );
}

void ClassifierListPage::slotListItemModified()
{
     if (!m_bSigWaiting) {
         return;
    }
    //is this safe???
    UMLClassifierListItem* object = const_cast<UMLClassifierListItem*>(dynamic_cast<const UMLClassifierListItem*>(sender()));
    if (object == NULL)
        return;
    QListWidgetItem* item = m_pItemListLB->currentItem();
    item->setText(object->toString(Uml::st_SigNoVis));
    m_pItemListLB->setCurrentItem(item);
    m_bSigWaiting = false;
}

void ClassifierListPage::deleteMenu()
{
    if (m_pMenu) {
        m_pMenu->hide();
        disconnect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupMenuSel(QAction*)));
        delete m_pMenu;
        m_pMenu = 0;
    }
}

void ClassifierListPage::slotRightButtonPressed(const QPoint& pos)
{
    ListPopupMenu::Menu_Type type = ListPopupMenu::mt_Undefined;
    if (m_pItemListLB->itemAt(pos)) { //pressed on a list item
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
            uWarning() << "unknown type in ClassifierListPage";
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
            uWarning() << "unknown type in ClassifierListPage";
        }
    }
    deleteMenu();
    m_pMenu = new ListPopupMenu(this, type);

    m_pMenu->popup(mapToGlobal(pos) + QPoint(0, 40));
    connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotPopupMenuSel(QAction*)));
}

void ClassifierListPage::slotPopupMenuSel(QAction* action)
{
    ListPopupMenu::Menu_Type id = m_pMenu->getMenuType(action);
    switch (id) {
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
        {
            int currentItemIndex = m_pItemListLB->currentRow();
            if ( currentItemIndex == -1 )
                return;
            UMLClassifierListItem* listItem = getItemList().at( currentItemIndex );
            if (!listItem && id != ListPopupMenu::mt_New_Attribute) {
                uDebug() << "can't find att from selection";
                return;
            }
            m_bSigWaiting = true;
            m_pDoc->renameChildUMLObject(listItem);
        }
        break;

    case ListPopupMenu::mt_Properties:
        slotProperties();
        break;

    default:
        uDebug() << "Menu_Type " << id << " not implemented";
    }
}

void ClassifierListPage::printItemList(const QString &prologue)
{
#ifdef VERBOSE_DEBUGGING
    UMLClassifierListItem* item;
    QString buf;
    UMLClassifierListItemList itemList = getItemList();
    for (UMLClassifierListItemListIt it(itemList); it.hasNext(); ) {
        item = it.next();
        buf.append(' ' + item->getName());
    }
    uDebug() << prologue << buf;
#endif
}

void ClassifierListPage::slotTopClicked()
{
    int count = m_pItemListLB->count();
    int index = m_pItemListLB->currentRow();
    //shouldn't occur, but just in case
    if( count <= 1 || index <= 0 )
        return;
    m_pOldListItem = NULL;

    //swap the text around in the ListBox
    QString currentString = m_pItemListLB->item(index)->text();
    m_pItemListLB->takeItem( index );
    m_pItemListLB->insertItem(0, currentString);
    //set the moved item selected
    QListWidgetItem* item = m_pItemListLB->item( 0 );
    m_pItemListLB->setCurrentItem(item);

    //now change around in the list
    printItemList("itemList before change: ");
    UMLClassifierListItem* currentAtt = getItemList().at(index);
    // NB: The index in the m_pItemListLB is not necessarily the same
    //     as the index in the UMLClassifier::m_List.
    //     Reason: getItemList() returns only a subset of all entries
    //     in UMLClassifier::m_List.
    takeItem(currentAtt, true, index);  // now we index the UMLClassifier::m_List
    uDebug() << currentAtt->getName() << ": peer index in UMLCanvasItem::m_List is " << index;
    addClassifier(currentAtt, 0);
    printItemList("itemList after change: ");
    slotClicked(item);
}

void ClassifierListPage::slotUpClicked()
{
    int count = m_pItemListLB->count();
    int index = m_pItemListLB->currentRow();
    //shouldn't occur, but just in case
    if ( count <= 1 || index <= 0 )
        return;
    m_pOldListItem = NULL;

    //swap the text around in the ListBox
    QString aboveString = m_pItemListLB->item(index - 1)->text();
    QString currentString = m_pItemListLB->item(index)->text();
    m_pItemListLB->item(index - 1)->setText(currentString);
    m_pItemListLB->item(index)->setText(aboveString);
    //set the moved item selected
    QListWidgetItem* item = m_pItemListLB->item( index - 1 );
    m_pItemListLB->setCurrentItem(item);

    //now change around in the list
    printItemList("itemList before change: ");
    UMLClassifierListItem* currentAtt = getItemList().at(index);
    // NB: The index in the m_pItemListLB is not necessarily the same
    //     as the index in the UMLClassifier::m_List.
    //     Reason: getItemList() returns only a subset of all entries
    //     in UMLClassifier::m_List.
    takeItem(currentAtt, true, index);  // now we index the UMLClassifier::m_List
    uDebug() << currentAtt->getName() << ": peer index in UMLCanvasItem::m_List is " << index;
    if (index == -1)
        index = 0;
    addClassifier(currentAtt, index);
    printItemList("itemList after change: ");
    slotClicked(item);
}

void ClassifierListPage::slotDownClicked()
{
    int count = m_pItemListLB->count();
    int index = m_pItemListLB->currentRow();
    //shouldn't occur, but just in case
    if ( count <= 1 || index >= count - 1 || index == -1 )
        return;
    m_pOldListItem = NULL;

    //swap the text around in the ListBox
    QString belowString = m_pItemListLB->item(index + 1)->text();
    QString currentString = m_pItemListLB->item(index)->text();
    m_pItemListLB->item(index + 1)->setText(currentString);
    m_pItemListLB->item(index)->setText(belowString);
    //set the moved item selected
    QListWidgetItem* item = m_pItemListLB->item( index + 1 );
    m_pItemListLB->setCurrentItem(item);
    //now change around in the list
    printItemList("itemList before change: ");
    UMLClassifierListItem* currentAtt = getItemList().at(index);
    // NB: The index in the m_pItemListLB is not necessarily the same
    //     as the index in the UMLClassifier::m_List.
    //     Reason: getItemList() returns only a subset of all entries
    //     in UMLClassifier::m_List.
    takeItem(currentAtt, false, index);  // now we index the UMLClassifier::m_List
    uDebug() << currentAtt->getName() << ": peer index in UMLCanvasItem::m_List is " << index;
    if (index != -1)
        index++;   // because we want to go _after_ the following peer item
    addClassifier(currentAtt, index);
    printItemList("itemList after change: ");
    slotClicked(item);
}

void ClassifierListPage::slotBottomClicked()
{
    int count = m_pItemListLB->count();
    int index = m_pItemListLB->currentRow();
    //shouldn't occur, but just in case
    if ( count <= 1 || index >= count - 1 || index == -1)
        return;
    m_pOldListItem = NULL;

    //swap the text around in the ListBox
    QString currentString = m_pItemListLB->item(index)->text();
    m_pItemListLB->takeItem(index);
    m_pItemListLB->insertItem( m_pItemListLB->count(), currentString);
    //set the moved item selected
    QListWidgetItem* item = m_pItemListLB->item( m_pItemListLB->count() - 1 );
    m_pItemListLB->setCurrentItem(item);

    //now change around in the list
    printItemList("itemList before change: ");
    UMLClassifierListItem* currentAtt = getItemList().at(index);
    // NB: The index in the m_pItemListLB is not necessarily the same
    //     as the index in the UMLClassifier::m_List.
    //     Reason: getItemList() returns only a subset of all entries
    //     in UMLClassifier::m_List.
    takeItem(currentAtt, false, index);  // now we index the UMLClassifier::m_List
    uDebug() << currentAtt->getName() << ": peer index in UMLCanvasItem::m_List is " << index;
    addClassifier(currentAtt, getItemList().count());
    printItemList("itemList after change: ");
    slotClicked(item);
}

void ClassifierListPage::slotDoubleClick( QListWidgetItem* item )
{
    if ( !item )
        return;

    UMLClassifierListItem* listItem  = getItemList().at( m_pItemListLB->row( item ) );
    if ( !listItem ) {
        uDebug() << "can't find att from selection";
        return;
    }

    m_bSigWaiting = true;
    if ( listItem->showPropertiesDialog(this) ) {
        m_pItemListLB->item(m_pItemListLB->row(item))->setText(listItem->toString(Uml::st_SigNoVis));
    }
}

void ClassifierListPage::slotDelete()
{
    int currentItemIndex = m_pItemListLB->currentRow();

    // index is -1 . Quit
    if ( currentItemIndex==-1 )
        return;

    UMLClassifierListItem* selectedItem = getItemList().at( currentItemIndex );
    //should really wait for signal back
    //but really shouldn't matter
    m_pDoc->removeUMLObject(selectedItem);
    m_pItemListLB->takeItem( m_pItemListLB->currentRow());
    m_pOldListItem = 0;
    slotClicked(NULL);
}

void ClassifierListPage::slotProperties()
{
    saveCurrentItemDocumentation();
    slotDoubleClick( m_pItemListLB->currentItem() );
}

void ClassifierListPage::slotNewListItem()
{
    saveCurrentItemDocumentation();
    m_bSigWaiting = true;
    m_pLastObjectCreated = Object_Factory::createChildObject(m_pClassifier, m_itemType);
    if ( m_pLastObjectCreated == NULL )
        m_bSigWaiting = false;
}

void ClassifierListPage::saveCurrentItemDocumentation()
{
    int currentItemIndex = m_pItemListLB->currentRow();

    // index is not in range, quit
    if ( currentItemIndex < 0 || currentItemIndex >= getItemList().count() )
        return;

    UMLClassifierListItem* selectedItem = getItemList().at( currentItemIndex );
    if (selectedItem) {
        selectedItem->setDoc( m_pDocTE->toPlainText() );
        if (m_itemType == ot_Operation) {
            dynamic_cast<UMLOperation*>(selectedItem)->setSourceCode( m_pCodeTE->toPlainText() );
        }
    }
}

UMLClassifierListItemList ClassifierListPage::getItemList()
{
    return m_pClassifier->getFilteredList(m_itemType);
}

bool ClassifierListPage::addClassifier(UMLClassifierListItem* listitem, int position)
{
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
            uWarning() << "unknown type in ClassifierListPage";
            return false;
        }
    }
    uError() << "unable to handle listitem type in current state";
    return false;
}

bool ClassifierListPage::takeItem(UMLClassifierListItem* listItem,
                                  bool seekPeerBefore, int &peerIndex)
{
    int wasAtIndex = m_pClassifier->takeItem(listItem);
    if (wasAtIndex == -1)
        return false;
    qApp->processEvents();
    peerIndex = -1;
    const Uml::Object_Type seekType = listItem->getBaseType();
    UMLObjectList listItems = m_pClassifier->subordinates();
    for (int i = 0; i < listItems.count(); ++i) {
        UMLObject *o = listItems.at(i);
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


int ClassifierListPage::calculateNewIndex(Uml::Object_Type /* ot */)
{
    return m_pItemListLB->count();
}

void ClassifierListPage::hideArrowButtons(bool hide)
{
    // if hide is true, we have to make state = false
    bool state = !hide;

    m_pTopArrowB->setVisible(state);
    m_pUpArrowB->setVisible(state);
    m_pDownArrowB->setVisible(state);
    m_pBottomArrowB->setVisible(state) ;
}


#include "classifierlistpage.moc"


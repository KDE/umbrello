/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2003-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "classifierlistpage.h"

#include "basictypes.h"
#include "classifierlistitem.h"
#include "codetextedit.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "classifier.h"
#include "enum.h"
#include "entity.h"
#include "attribute.h"
#include "listpopupmenu.h"
#include "operation.h"
#include "template.h"
#include "enumliteral.h"
#include "entityattribute.h"
#include "object_factory.h"

#include <kdialogbuttonbox.h>
#include <klocale.h>
#include <ktabwidget.h>
#include <ktextedit.h>

#include <QtGui/QApplication>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>

/**
 *  Sets up the ClassifierListPage.
 *  @param parent      The parent to the ClassAttPage.
 *  @param classifier  The Concept to display the properties of.
 *  @param doc         The UMLDoc document
 *  @param type        The type of listItem this handles
 */
ClassifierListPage::ClassifierListPage(QWidget* parent, UMLClassifier* classifier,
                                       UMLDoc* doc, UMLObject::ObjectType type)
  : QWidget(parent)
{
    m_itemType = type;
    m_bSigWaiting = false;
    m_doc = doc;
    m_pClassifier = classifier;
    m_pMenu = 0;

    setupPage();
}

/**
 *  Standard destructor.
 */
ClassifierListPage::~ClassifierListPage()
{
}

/**
 * Sets up the page.
 */
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
    mainLayout->addWidget(m_docGB);

    reloadItemListBox();

    enableWidgets(false);//disable widgets until an att is chosen
    m_pOldListItem = 0;

    connect(m_pItemListLB, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(slotActivateItem(QListWidgetItem*)));
    connect(m_pItemListLB, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotDoubleClick(QListWidgetItem*)));
    connect(m_pItemListLB, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotRightButtonPressed(QPoint)));

    connect(m_doc, SIGNAL(sigObjectCreated(UMLObject*)), this, SLOT(slotListItemCreated(UMLObject*)));

    connect(m_pTopArrowB, SIGNAL(clicked()), this, SLOT(slotTopClicked()));
    connect(m_pUpArrowB, SIGNAL(clicked()), this, SLOT(slotUpClicked()));
    connect(m_pDownArrowB, SIGNAL(clicked()), this, SLOT(slotDownClicked()));
    connect(m_pBottomArrowB, SIGNAL(clicked()), this, SLOT(slotBottomClicked()));
}

/**
 * Sets up the list group.
 * @param margin  The margin of the group.
 */
void ClassifierListPage::setupListGroup(int margin)
{
    QString typeName;
    QString newItemType;

    switch (m_itemType) {
    case UMLObject::ot_Attribute:
        typeName = i18n("Attributes");
        newItemType = i18n("N&ew Attribute...");
        break;
    case UMLObject::ot_Operation:
        typeName = i18n("Operations");
        newItemType = i18n("N&ew Operation...");
        break;
    case UMLObject::ot_Template:
        typeName = i18n("Templates");
        newItemType = i18n("N&ew Template...");
        break;
    case UMLObject::ot_EnumLiteral:
        typeName = i18n("Enum Literals");
        newItemType = i18n("N&ew Enum Literal...");
        break;
    case UMLObject::ot_EntityAttribute:
        typeName = i18n("Entity Attributes");
        newItemType = i18n("N&ew Entity Attribute...");
        break;
    case UMLObject::ot_EntityConstraint:
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

/**
 * Sets up the move up/down buttons.
 * @param parentLayout  The parent layout to which this group belongs.
 */
void ClassifierListPage::setupMoveButtons(QHBoxLayout* parentLayout)
{
    QVBoxLayout* buttonLayout = new QVBoxLayout();
    parentLayout->addItem(buttonLayout);

    m_pTopArrowB = new QToolButton(m_pItemListGB);
    m_pTopArrowB->setArrowType(Qt::UpArrow);
    m_pTopArrowB->setEnabled(false);
    m_pTopArrowB->setToolTip(i18n("Move selected item to the top"));
    buttonLayout->addWidget(m_pTopArrowB);

    m_pUpArrowB = new QToolButton(m_pItemListGB);
    m_pUpArrowB->setArrowType(Qt::UpArrow);
    m_pUpArrowB->setEnabled(false);
    m_pUpArrowB->setToolTip(i18n("Move selected item up"));
    buttonLayout->addWidget(m_pUpArrowB);

    m_pDownArrowB = new QToolButton(m_pItemListGB);
    m_pDownArrowB->setArrowType(Qt::DownArrow);
    m_pDownArrowB->setEnabled(false);
    m_pDownArrowB->setToolTip(i18n("Move selected item down"));
    buttonLayout->addWidget(m_pDownArrowB);

    m_pBottomArrowB = new QToolButton(m_pItemListGB);
    m_pBottomArrowB->setArrowType(Qt::DownArrow);
    m_pBottomArrowB->setEnabled(false);
    m_pBottomArrowB->setToolTip(i18n("Move selected item to the bottom"));
    buttonLayout->addWidget(m_pBottomArrowB);
}

/**
 * Sets up the action buttons.
 * @param itemType      The item type.
 * @param parentLayout  The parent layout to which this group belongs.
 */
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

/**
 * Sets up the documentation group.
 * @param margin  The margin of the group.
 */
void ClassifierListPage::setupDocumentationGroup(int margin)
{
    m_docGB = new QGroupBox(i18n("Documentation"), this);
    QVBoxLayout* docLayout = new QVBoxLayout(m_docGB);
    docLayout->setSpacing(10);
    docLayout->setMargin(margin);
    if (m_itemType == UMLObject::ot_Operation) {
        m_docTE = new KTextEdit();
        m_pCodeTE = new CodeTextEdit();
        KTabWidget* tabWidget = new KTabWidget();
        tabWidget->addTab(m_docTE, i18n("Comment"));
        tabWidget->addTab(m_pCodeTE, i18n("Source Code"));
        docLayout->addWidget(tabWidget);
    }
    else {
        m_docTE = new KTextEdit();
        docLayout->addWidget(m_docTE);
    }
}

/**
 * Loads the Item List Box.
 */
void ClassifierListPage::reloadItemListBox()
{
    UMLClassifierListItemList itemList(getItemList());

    // remove any items if present
    m_pItemListLB->clear();

    // add each item in the list to the ListBox and connect each item modified signal
    // to the ListItemModified slot in this class
    foreach (UMLClassifierListItem* listItem, itemList ) {
        m_pItemListLB->addItem(listItem->toString(Uml::SignatureType::SigNoVis));
        connect( listItem, SIGNAL(modified()), this, SLOT(slotListItemModified()) );
    }
}

/**
 * Set the state of the widgets on the page with the given value.
 * @param  state   The state to set the widgets as.
 */
void ClassifierListPage::enableWidgets(bool state)
{
    m_docTE->setEnabled( state );
    if (m_itemType == UMLObject::ot_Operation) {
        m_pCodeTE->setEnabled( state );
    }
    //if disabled clear contents
    if ( !state ) {
        m_docTE->setText( "" );
        if (m_itemType == UMLObject::ot_Operation) {
            m_pCodeTE->setPlainText( "" );
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
    if ( m_pItemListLB->count() == 1 || index == -1 ) {
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

/**
 * Called whenever the list item needs to be activated
 * calls enableWidgets().
 */
void ClassifierListPage::slotActivateItem(QListWidgetItem* item)
{
    //if not first time an item is highlighted
    //save old highlighted item first
    if (m_pOldListItem) {
        m_pOldListItem->setDoc( m_docTE->toPlainText() );
        if (m_itemType == UMLObject::ot_Operation) {
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
    UMLClassifierListItem* listItem = NULL;
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
        m_docTE->setText( listItem->doc() );
        if (m_itemType == UMLObject::ot_Operation) {
            m_pCodeTE->setPlainText( dynamic_cast<UMLOperation*>(listItem)->getSourceCode() );
        }
        enableWidgets(true);
        m_pOldListItem = listItem;
    }
}

/**
 *  Will move information from the dialog into the object.
 *  Call when the ok or apply button is pressed.
 */
void ClassifierListPage::updateObject()
{
    saveCurrentItemDocumentation();
    QListWidgetItem* i = m_pItemListLB->currentItem();
    slotActivateItem(i);
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

    QString itemStr = listItem->toString(Uml::SignatureType::SigNoVis);
    // already in list?
    QList<QListWidgetItem*> foundItems = m_pItemListLB->findItems(itemStr, Qt::MatchExactly);
    int index = -1;
    if (foundItems.empty()) {
        index = m_pItemListLB->count();
        m_pItemListLB->insertItem(index, itemStr);
    }
    else {
        index = m_pItemListLB->row(foundItems[0]);
    }
    m_bSigWaiting = false;

    // now select the new item, so that the user can go on adding doc or calling
    // the property dialog
    if (index > -1) {
        m_pItemListLB->setCurrentItem(m_pItemListLB->item(index));
        slotActivateItem(m_pItemListLB->item(index));
        connect(object, SIGNAL(modified()), this, SLOT(slotListItemModified()));
    }
}

void ClassifierListPage::slotListItemModified()
{
     if (!m_bSigWaiting) {
         return;
    }
    UMLClassifierListItem* object = dynamic_cast<UMLClassifierListItem*>(sender());
    if (object) {
        QListWidgetItem* item = m_pItemListLB->currentItem();
        item->setText(object->toString(Uml::SignatureType::SigNoVis));
        m_pItemListLB->setCurrentItem(item);
        m_bSigWaiting = false;
    }
}

/**
 * Hide menu and free all its resources.
 */
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
    ListPopupMenu::MenuType type = ListPopupMenu::mt_Undefined;
    if (m_pItemListLB->itemAt(pos)) { //pressed on a list item
        if (m_itemType == UMLObject::ot_Attribute) {
            type = ListPopupMenu::mt_Attribute_Selected;
        } else if (m_itemType == UMLObject::ot_Operation) {
            type = ListPopupMenu::mt_Operation_Selected;
        } else if (m_itemType == UMLObject::ot_Template) {
            type = ListPopupMenu::mt_Template_Selected;
        } else if (m_itemType == UMLObject::ot_EnumLiteral) {
            type = ListPopupMenu::mt_EnumLiteral_Selected;
        } else if (m_itemType == UMLObject::ot_EntityAttribute) {
            type = ListPopupMenu::mt_EntityAttribute_Selected;
        } else {
            uWarning() << "unknown type in ClassifierListPage";
        }
    } else { //pressed into fresh air
        if (m_itemType == UMLObject::ot_Attribute) {
            type = ListPopupMenu::mt_New_Attribute;
        } else if (m_itemType == UMLObject::ot_Operation) {
            type = ListPopupMenu::mt_New_Operation;
        } else if (m_itemType == UMLObject::ot_Template) {
            type = ListPopupMenu::mt_New_Template;
        } else if (m_itemType == UMLObject::ot_EnumLiteral) {
            type = ListPopupMenu::mt_New_EnumLiteral;
        } else if (m_itemType == UMLObject::ot_EntityAttribute) {
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

/**
 * Called when an item is selected in a right click menu.
 */
void ClassifierListPage::slotPopupMenuSel(QAction* action)
{
    ListPopupMenu::MenuType id = m_pMenu->getMenuType(action);
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
                uDebug() << "can not find att from selection";
                return;
            }
            m_bSigWaiting = true;
            m_doc->renameChildUMLObject(listItem);
        }
        break;

    case ListPopupMenu::mt_Properties:
        slotProperties();
        break;

    default:
        uDebug() << "MenuType " << ListPopupMenu::toString(id) << " not implemented";
    }
}

/**
 * Utility for debugging, prints the current item list.
 * Only effective if VERBOSE_DEBUGGING is defined.
 */
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
#else
    Q_UNUSED(prologue);
#endif
}

/**
 * Moves selected attribute to the top of the list.
 */
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
    uDebug() << currentAtt->name() << ": peer index in UMLCanvasItem::m_List is " << index;
    addClassifier(currentAtt, 0);
    printItemList("itemList after change: ");
    slotActivateItem(item);
}

/**
 * Moves selected attribute up in list.
 */
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
    uDebug() << currentAtt->name() << ": peer index in UMLCanvasItem::m_List is " << index;
    if (index == -1)
        index = 0;
    addClassifier(currentAtt, index);
    printItemList("itemList after change: ");
    slotActivateItem(item);
}

/**
 * Moved selected attribute down in list.
 */
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
    uDebug() << currentAtt->name() << ": peer index in UMLCanvasItem::m_List is " << index;
    if (index != -1)
        index++;   // because we want to go _after_ the following peer item
    addClassifier(currentAtt, index);
    printItemList("itemList after change: ");
    slotActivateItem(item);
}

/**
 * Moved selected attribute to the bottom of the list.
 */
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
    uDebug() << currentAtt->name() << ": peer index in UMLCanvasItem::m_List is " << index;
    addClassifier(currentAtt, getItemList().count());
    printItemList("itemList after change: ");
    slotActivateItem(item);
}

/**
 * Shows properties dialog for the attribute clicked on.
 */
void ClassifierListPage::slotDoubleClick( QListWidgetItem* item )
{
    if ( !item ) {
        return;
    }

    UMLClassifierListItem* listItem  = getItemList().at( m_pItemListLB->row( item ) );
    if ( !listItem ) {
        uDebug() << "can not find att from selection";
        return;
    }

    m_bSigWaiting = true;
    if ( listItem->showPropertiesDialog(this) ) {
        m_pItemListLB->item(m_pItemListLB->row(item))->setText(listItem->toString(Uml::SignatureType::SigNoVis));
    }
}

/**
 * Removes currently seleted attribute.
 */
void ClassifierListPage::slotDelete()
{
    int currentItemIndex = m_pItemListLB->currentRow();

    if ( currentItemIndex > -1 ) {

        // do this first
        delete m_pItemListLB->takeItem(currentItemIndex);

        UMLClassifierListItem* selectedItem = getItemList().at(currentItemIndex);
        //should really wait for signal back
        //but really shouldn't matter
        m_doc->removeUMLObject(selectedItem);
        m_pOldListItem = 0;

        slotActivateItem(NULL);
    }
}

/**
 * Shows properties dialog for currently selected attribute.
 */
void ClassifierListPage::slotProperties()
{
    saveCurrentItemDocumentation();
    slotDoubleClick( m_pItemListLB->currentItem() );
}

/**
 * Shows dialog for new attribute.
 */
void ClassifierListPage::slotNewListItem()
{
    saveCurrentItemDocumentation();
    m_bSigWaiting = true;
    m_pLastObjectCreated = Object_Factory::createChildObject(m_pClassifier, m_itemType);
    if ( m_pLastObjectCreated == NULL ) {
        m_bSigWaiting = false;
    }
}

/**
 * Saves the documentation for the currently selected item.
 */
void ClassifierListPage::saveCurrentItemDocumentation()
{
    int currentItemIndex = m_pItemListLB->currentRow();

    // index is not in range, quit
    if ( currentItemIndex < 0 || currentItemIndex >= getItemList().count() ) {
        return;
    }

    UMLClassifierListItem* selectedItem = getItemList().at( currentItemIndex );
    if (selectedItem) {
        selectedItem->setDoc( m_docTE->toPlainText() );
        if (m_itemType == UMLObject::ot_Operation) {
            dynamic_cast<UMLOperation*>(selectedItem)->setSourceCode( m_pCodeTE->toPlainText() );
        }
    }
}

/**
 * Get classifier list items.
 */
UMLClassifierListItemList ClassifierListPage::getItemList()
{
    return m_pClassifier->getFilteredList(m_itemType);
}

/**
 * Attempts to add classifier to the appropriate list.
 * @param classifier   Pointer to the classifier to add.
 * @param position     Index at which to insert into the list.
 * @return             true if the classifier could be added
 *
 */
bool ClassifierListPage::addClassifier(UMLClassifierListItem* listitem, int position)
{
    switch (m_itemType) {
    case UMLObject::ot_Attribute: {
            UMLAttribute *att = dynamic_cast<UMLAttribute*>(listitem);
            return m_pClassifier->addAttribute(att, NULL, position);
        }
    case UMLObject::ot_Operation: {
            UMLOperation *op = dynamic_cast<UMLOperation*>(listitem);
            return m_pClassifier->addOperation(op, position);
        }
    case UMLObject::ot_Template: {
            UMLTemplate* t = dynamic_cast<UMLTemplate*>(listitem);
            return m_pClassifier->addTemplate(t, position);
        }
    case UMLObject::ot_EnumLiteral: {
            UMLEnum* c = dynamic_cast<UMLEnum*>(m_pClassifier);
            if (c) {
                return c->addEnumLiteral(dynamic_cast<UMLEnumLiteral*>(listitem), position);
            }
            break;
        }
    case UMLObject::ot_EntityAttribute: {
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

/**
 * Take a classifier's subordinate item.
 * Ownership of the classifier list item is transferred to the caller.
 * @param listitem        UMLClassifierListItem to take.
 * @param seekPeerBefore  True if a peer index should be sought which
 *                        is smaller than the current listitem's index.
 * @param peerIndex       Return value: Index in the UMLClassifier's
 *                        item list at which a peer item, i.e. another
 *                        UMLClassifierListItem of the same type as
 *                        listItem, is found.  If no such item exists
 *                        then return -1.
 * @return                True for success.
 */
bool ClassifierListPage::takeItem(UMLClassifierListItem* listItem,
                                  bool seekPeerBefore, int &peerIndex)
{
    int wasAtIndex = m_pClassifier->takeItem(listItem);
    if (wasAtIndex == -1)
        return false;
    qApp->processEvents();
    peerIndex = -1;
    const UMLObject::ObjectType seekType = listItem->baseType();
    UMLObjectList listItems = m_pClassifier->subordinates();
    for (int i = 0; i < listItems.count(); ++i) {
        UMLObject *o = listItems.at(i);
        if (seekPeerBefore) {
            if (i >= wasAtIndex)
                break;
            if (o->baseType() == seekType)
                peerIndex = i;
        } else {    // seekPeerAfter
            if (i < wasAtIndex)
                continue;
            if (o->baseType() == seekType) {
                peerIndex = i;
                break;
            }
        }
    }
    return true;
}

/**
 * Sets the visibility of the arrow buttons.
 * @param hide true hides the arrow buttons
 */
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


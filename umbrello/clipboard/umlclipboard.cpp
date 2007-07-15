/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlclipboard.h"

// qt/kde includes
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

// local includes
#include "umldrag.h"
#include "idchangelog.h"
#include "../associationwidget.h"
#include "../attribute.h"
#include "../classifier.h"
#include "../floatingtextwidget.h"
#include "../operation.h"
#include "../umldoc.h"
#include "../umllistview.h"
#include "../umllistviewitem.h"
#include "../umlobjectlist.h"
#include "../umlview.h"
#include "../umlwidget.h"
#include "../uml.h"
#include "../model_utils.h"

UMLClipboard::UMLClipboard() {
    m_type = clip1;
}

UMLClipboard::~UMLClipboard() {
}

QMimeSource* UMLClipboard::copy(bool fromView/*=false*/) {
    //Clear previous copied data
    m_AssociationList.clear();
    m_ItemList.clear();
    m_ObjectList.clear();
    m_ViewList.clear();

    UMLDrag *data = 0;
    QPixmap* png = 0;

    UMLListView * listView = UMLApp::app()->getListView();
    UMLListViewItemList selectedItems;
    selectedItems.setAutoDelete(false);

    if(fromView) {
        m_type = clip4;
        UMLView *view = UMLApp::app()->getCurrentView();
        view->checkSelections();
        if(!view->getSelectedWidgets(m_WidgetList)) {
            return 0;
        }
        //if there is no selected widget then there is no copy action
        if(!m_WidgetList.count()) {
            return 0;
        }
        m_AssociationList = view->getSelectedAssocs();
        view->copyAsImage(png);

    } else { //if the copy action is being performed from the ListView
        if(!listView->getSelectedItems(selectedItems)) {
            return 0;
        }
        //Set What type of copy operation are we performing and
        //also fill m_ViewList with all the selected Diagrams
        setCopyType(selectedItems);

        //if we are copying a diagram or part of a diagram, select the items
        //on the ListView that correspond to a UseCase, Actor or Concept
        //in the Diagram
        if(m_type == clip2) {
            //Fill the member lists with all the object and stuff to be copied
            //to the clipboard
            selectedItems.clear();
            //For each selected view select all the Actors, USe Cases and Concepts
            //widgets in the ListView
            for (UMLViewListIt vit(m_ViewList); vit.current(); ++vit) {
                UMLObjectList objects = vit.current()->getUMLObjects();
                for (UMLObjectListIt oit(objects); oit.current(); ++oit) {
                    UMLObject *o = oit.current();
                    UMLListViewItem *item = listView->findUMLObject(o);
                    if(item) {
                        listView->setSelected(item, true);
                    }
                }
            }
            if(!listView->getSelectedItems(selectedItems)) {
                return 0;
            }
        }
        if(!fillSelectionLists(selectedItems)) {
            return 0;
        }
    }
    int i =0;
    switch(m_type) {
    case clip1:
        data = new UMLDrag(m_ObjectList);
        break;
    case clip2:
        data = new UMLDrag(m_ObjectList, m_ItemList, m_ViewList);
        break;
    case clip3:
        data = new UMLDrag(m_ItemList);
        break;
    case clip4:
        if(png) {
            UMLView *view = UMLApp::app()->getCurrentView();
            data = new UMLDrag(m_ObjectList, m_WidgetList,
                               m_AssociationList, *png, view->getType());
        } else {
            return 0;
        }
        break;
    case clip5:
        data = new UMLDrag(m_ObjectList, i);
        // The int i is used to differentiate
        // which UMLDrag constructor gets called.
        break;
    }

    return (QMimeSource*)data;
}

bool UMLClipboard::paste(QMimeSource* data) {
    UMLDoc *doc = UMLApp::app()->getDocument();
    bool result = false;
    doc->beginPaste();
    switch(UMLDrag::getCodingType(data)) {
    case 1:
        result = pasteClip1(data);
        break;
    case 2:
        result = pasteClip2(data);
        break;
    case 3:
        result = pasteClip3(data);
        break;
    case 4:
        result = pasteClip4(data);
        break;
    case 5:
        result = pasteClip5(data);
        break;
    default:
        break;
    }
    doc->endPaste();
    return result;
}

bool UMLClipboard::fillSelectionLists(UMLListViewItemList& SelectedItems) {
    UMLListViewItemListIt it(SelectedItems);
    UMLListViewItem* item = 0;
    Uml::ListView_Type type;
    switch(m_type) {
    case clip4:
        break;
    case clip3:
        for ( ; it.current(); ++it ) {
            item = (UMLListViewItem*)it.current();
            type = item->getType();
            if ( !Model_Utils::typeIsClassifierList(type) ) {
                m_ItemList.append(item);
                insertItemChildren(item, SelectedItems);
                //Because it is being called when m_type is 3
                //it will insert only child empty folders of other folders.
                //If a child folder
                //is not empty that means m_type wouldn't be 3 because if a folder is
                //selected then its complete contents are treated as if
                //they were selected
            }
        }
        break;
    case clip2:
    case clip1:
        for ( ; it.current(); ++it ) {
            item = (UMLListViewItem*)it.current();
            type = item->getType();
            if ( !Model_Utils::typeIsClassifierList(type) ) {

                m_ItemList.append(item);

                if ( Model_Utils::typeIsCanvasWidget(type) ) {
                    m_ObjectList.append(item->getUMLObject());
                }
                insertItemChildren(it.current(), SelectedItems);
            }
        }
        break;
    case clip5:
        for ( ; it.current(); ++it ) {
            item = (UMLListViewItem*)it.current();
            type = item->getType();
            if( Model_Utils::typeIsClassifierList(type) ) {
                m_ItemList.append(item);
                m_ObjectList.append(item->getUMLObject());

            } else {
                return false;
            }
        }
        break;
    }

    return true;
}

void UMLClipboard::setCopyType(UMLListViewItemList& SelectedItems) {
    bool withDiagrams = false; //If the selection includes diagrams
    bool withObjects = false; //If the selection includes objects
    bool onlyAttsOps = false; //If the selection only includes Attributes and/or Operations
    UMLListViewItemListIt it(SelectedItems);
    for ( ; it.current(); ++it ) {
        checkItemForCopyType(it.current(), withDiagrams, withObjects, onlyAttsOps);
    }
    if(onlyAttsOps) {
        m_type = clip5;
    } else if(withDiagrams) {
        m_type = clip2;
    } else if(withObjects) {
        m_type = clip1;
    } else {
        m_type = clip3;
    }
}

void UMLClipboard::checkItemForCopyType(UMLListViewItem* Item, bool & WithDiagrams, bool &WithObjects,
                                        bool &OnlyAttsOps) {
    if(!Item) {
        return;
    }
    UMLDoc *doc = UMLApp::app()->getDocument();
    OnlyAttsOps = true;
    UMLView * view = 0;
    UMLListViewItem * child = 0;
    Uml::ListView_Type type = Item->getType();
    if ( Model_Utils::typeIsCanvasWidget(type) ) {
        WithObjects = true;
        OnlyAttsOps = false;
    } else if ( Model_Utils::typeIsDiagram(type) ) {
        WithDiagrams = true;
        OnlyAttsOps = false;
        view = doc->findView( Item->getID() );
        m_ViewList.append( view );
    } else if ( Model_Utils::typeIsFolder(type) ) {
        OnlyAttsOps = false;
        if(Item->childCount()) {
            child = (UMLListViewItem*)Item->firstChild();
            while(child) {
                checkItemForCopyType(child, WithDiagrams, WithObjects, OnlyAttsOps);
                child = (UMLListViewItem*)child->nextSibling();
            }
        }
    }
}

/** Adds the children of a UMLListViewItem to m_ItemList */
bool UMLClipboard::insertItemChildren(UMLListViewItem * Item, UMLListViewItemList& SelectedItems) {
    if(Item->childCount()) {
        UMLListViewItem * child = (UMLListViewItem*)Item->firstChild();
        int type;
        while(child) {
            m_ItemList.append(child);
            type = child->getType();
            if(type == Uml::lvt_Actor || type == Uml::lvt_UseCase || type == Uml::lvt_Class) {
                m_ObjectList.append(child->getUMLObject());
            }
            // If the child is selected, remove it from the list of selected items
            // otherwise it will be inserted twice in m_ObjectList.
            if(child->isSelected()) {
                SelectedItems.remove(SelectedItems.find(child) );
            }
            insertItemChildren(child, SelectedItems);
            child = (UMLListViewItem*)child->nextSibling();
        }
    }
    return true;
}

bool UMLClipboard::pasteChildren(UMLListViewItem *parent, IDChangeLog *chgLog) {
    if (!parent) {
        kWarning() << "Paste Children Error, parent missing" << endl;
        return false;
    }
    UMLDoc *doc = UMLApp::app()->getDocument();
    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem *childItem = static_cast<UMLListViewItem*>(parent->firstChild());
    while (childItem) {
        Uml::IDType oldID = childItem->getID();
        Uml::IDType newID = chgLog->findNewID(oldID);
        UMLListViewItem *shouldNotExist = listView->findItem(newID);
        if (shouldNotExist) {
            kError() << "UMLClipboard::pasteChildren: new list view item " << ID2STR(newID)
            << " already exists (internal error)" << endl;
            childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
            continue;
        }
        UMLObject *newObj = doc->findObjectById(newID);
        if (newObj) {
            kDebug() << "UMLClipboard::pasteChildren: adjusting lvitem(" << ID2STR(oldID)
            << ") to new UMLObject(" << ID2STR(newID) << ")" << endl;
            childItem->setUMLObject(newObj);
            childItem->setText(newObj->getName());
        } else {
            kDebug() << "UMLClipboard::pasteChildren: no UMLObject found for lvitem "
            << ID2STR(newID) << endl;
        }
        childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
    }
    return true;
}

/** Cleans the list of associations taking out the ones that point to an object
    not in m_ObjectList. */
void UMLClipboard::CleanAssociations(AssociationWidgetList& associations) {
    AssociationWidgetListIt it(associations);
    AssociationWidget* assoc = it.current();

    while (assoc) {
        ++it;
        assoc = it.current();
    }
}

/** If clipboard has mime type application/x-uml-clip1,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip1(QMimeSource* data) {
    UMLObjectList objects;
    if (! UMLDrag::decodeClip1(data, objects)) {
        return false;
    }
    UMLListView *lv = UMLApp::app()->getListView();
    if ( !lv->startedCopy() )
        return true;
    lv->setStartedCopy(false);
    /* If we get here we are pasting after a Copy and need to
    // paste possible children.
    UMLListViewItem* itemdata = 0;
    UMLListViewItemListIt it(itemdatalist);
    while ( (itemdata=it.current()) != 0 ) {
        if(itemdata -> childCount()) {
                if(!pasteChildren(itemdata, idchanges)) {
                        return false;
                }
        }
        ++it;
    }
     */
    return true;
}

/** If clipboard has mime type application/x-uml-clip2,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip2(QMimeSource* data) {
    UMLDoc *doc = UMLApp::app()->getDocument();
    UMLListViewItemList itemdatalist;
    UMLObjectList objects;
    objects.setAutoDelete(false);
    UMLViewList         views;
    IDChangeLog* idchanges = 0;

    bool result = UMLDrag::decodeClip2(data, objects, itemdatalist, views);
    if(!result) {
        return false;
    }
    UMLObject *obj = 0;
    UMLObjectListIt object_it(objects);
    idchanges = doc->getChangeLog();
    if(!idchanges) {
        return false;
    }
    while ( (obj=object_it.current()) != 0 ) {
        ++object_it;
        if(!doc->assignNewIDs(obj)) {
            kDebug()<<"UMLClipboard: error adding umlobject"<<endl;
            return false;
        }
    }

    UMLView * pView = 0;
    UMLViewListIt view_it( views );
    while ( ( pView =view_it.current()) != 0 ) {
        ++view_it;
        if( !doc->addUMLView( pView ) ) {
            return false;
        }
    }

    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem* item = 0;
    UMLListViewItem* itemdata = 0;
    UMLListViewItemListIt it(itemdatalist);
    while ( (itemdata=it.current()) != 0 ) {
        item = listView->createItem(*itemdata, *idchanges);
        if(!item) {
            return false;
        }
        if(itemdata -> childCount()) {
            if(!pasteChildren(item, idchanges)) {
                return false;
            }
        }
        ++it;
    }

    return result;
}

/** If clipboard has mime type application/x-uml-clip3,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip3(QMimeSource* data) {
    UMLDoc *doc = UMLApp::app()->getDocument();
    UMLListViewItemList itemdatalist;
    UMLListViewItem* item = 0;
    UMLListViewItem* itemdata = 0;
    IDChangeLog* idchanges = doc->getChangeLog();

    if(!idchanges) {
        return false;
    }

    UMLListView *listView = UMLApp::app()->getListView();
    bool result = UMLDrag::decodeClip3(data, itemdatalist, listView);
    if(!result) {
        return false;
    }
    UMLListViewItemListIt it(itemdatalist);
    while ( (itemdata=it.current()) != 0 ) {
        item = listView->createItem(*itemdata, *idchanges);
        if(itemdata -> childCount()) {
            if(!pasteChildren(item, idchanges)) {
                return false;
            }
        }
        ++it;
    }

    return result;
}

/** If clipboard has mime type application/x-uml-clip4,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip4(QMimeSource* data) {
    UMLDoc *doc = UMLApp::app()->getDocument();

    UMLObjectList objects;
    objects.setAutoDelete(false);


    UMLWidgetList               widgets;
    widgets.setAutoDelete(false);

    AssociationWidgetList       assocs;
    assocs.setAutoDelete(false);

    IDChangeLog* idchanges = 0;

    Uml::Diagram_Type diagramType;

    if( !UMLDrag::decodeClip4(data, objects, widgets, assocs, diagramType) ) {
        return false;
    }

    if( diagramType != UMLApp::app()->getCurrentView()->getType() ) {
        if( !checkPasteWidgets(widgets) ) {
            assocs.setAutoDelete(true);
            assocs.clear();
            return false;
        }
    }
    UMLObjectListIt object_it(objects);
    idchanges = doc->getChangeLog();
    if(!idchanges) {
        return false;
    }
     //make sure the file we are pasting into has the objects
     //we need if there are widgets to be pasted
     UMLObject* obj = 0;
     while ( (obj=object_it.current()) != 0 ) {
         ++object_it;

        if(!doc->assignNewIDs(obj)) {
            return false;
        }

     }

    //now add any widget we are want to paste
    bool objectAlreadyExists = false;
    UMLView *currentView = UMLApp::app()->getCurrentView();
    currentView->beginPartialWidgetPaste();
    UMLWidget* widget =0;
    UMLWidgetListIt widget_it(widgets);
    while ( (widget=widget_it.current()) != 0 ) {
        ++widget_it;

        Uml::IDType oldId = widget->getID();
        Uml::IDType newId = idchanges->findNewID(oldId);
        if (currentView->findWidget(newId)) {
            kError() << "UMLClipboard::pasteClip4: widget (oldID=" << ID2STR(oldId)
                << ", newID=" << ID2STR(newId) << ") already exists in target view."
                << endl;
            widgets.remove(widget);
            delete widget;
            objectAlreadyExists = true;
        } else if (! currentView->addWidget(widget, true)) {
            currentView->endPartialWidgetPaste();
            return false;
        }
    }

    //now paste the associations
    AssociationWidget* assoc;
    AssociationWidgetListIt assoc_it(assocs);
    while ( (assoc=assoc_it.current()) != 0 ) {
        ++assoc_it;
        if (!currentView->addAssociation(assoc, true)) {
            currentView->endPartialWidgetPaste();
            return false;
        }
    }

    //Activate all the pasted associations and widgets
    currentView->activate();
    currentView->endPartialWidgetPaste();

    /*
    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem* item = 0;
    UMLListViewItem* itemdata = 0;
    UMLListViewItemListIt it(itemdatalist);
    while ( (itemdata=it.current()) != 0 ) {
        item = listView->createItem(*itemdata, *idchanges);
        if(!item) {
            return false;
        }
        if(itemdata -> childCount()) {
            if(!pasteChildren(item, idchanges)) {
                return false;
            }
        }
        ++it;
        }*/

    if (objectAlreadyExists) {
        pasteItemAlreadyExists();
    }
    return true;
}

/** If clipboard has mime type application/x-uml-clip5,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip5(QMimeSource* data) {
    UMLDoc *doc = UMLApp::app()->getDocument();
    UMLListView *listView = UMLApp::app()->getListView();
    UMLListViewItem* lvitem = dynamic_cast<UMLListViewItem *>( listView->currentItem() );
    if (!lvitem ||
            (lvitem->getType() != Uml::lvt_Class && lvitem->getType() != Uml::lvt_Interface)) {
        return false;
    }
    UMLClassifier *parent = dynamic_cast<UMLClassifier *>(lvitem->getUMLObject());
    if (parent == NULL) {
        kError() << "UMLClipboard::pasteClip5: parent is not a UMLClassifier"
        << endl;
        return false;
    }

    UMLObjectList objects;
    objects.setAutoDelete(false);
    IDChangeLog* idchanges = 0;
    bool result = UMLDrag::decodeClip5(data, objects, parent);

    if(!result) {
        return false;
    }

    UMLObject   *obj = 0;
    doc->setModified(true);
    idchanges = doc->getChangeLog();
    // Assume success if at least one child object could be pasted
    if (objects.count())
        result = false;

    for (UMLObjectListIt it(objects); (obj = it.current()) != NULL; ++it) {
        obj->setID(doc->assignNewID(obj->getID()));
        switch(obj->getBaseType()) {
        case Uml::ot_Attribute :
            {
                UMLObject *exist = parent->findChildObject(obj->getName(), Uml::ot_Attribute);
                if (exist) {
                    QString newName = parent->uniqChildName(Uml::ot_Attribute, obj->getName());
                    obj->setName(newName);
                }
                UMLAttribute *att = static_cast<UMLAttribute*>(obj);
                if (parent->addAttribute(att, idchanges)) {
                    result = true;
                } else {
                    kError() << "UMLClipboard::pasteClip5: " << parent->getName()
                        << "->addAttribute(" << att->getName() << ") failed" << endl;
                }
                break;
            }
        case Uml::ot_Operation :
            {
                UMLOperation *op = static_cast<UMLOperation*>(obj);
                UMLOperation *exist = parent->checkOperationSignature(op->getName(), op->getParmList());
                if (exist) {
                    QString newName = parent->uniqChildName(Uml::ot_Operation, obj->getName());
                    op->setName(newName);
                }
                if (parent->addOperation(op, idchanges)) {
                    result = true;
                } else {
                    kError() << "UMLClipboard::pasteClip5: " << parent->getName()
                        << "->addOperation(" << op->getName() << ") failed" << endl;
                }
                break;
            }
        default :
            kWarning() << "pasting unknown children type in clip type 5" << endl;
            return false;
        }
    }

    return result;
}

bool UMLClipboard::insertItemChildren( UMLListViewItem * item ) {
    if( item -> childCount() ) {
        UMLListViewItem * child =dynamic_cast<UMLListViewItem *>( item -> firstChild() );
        while( child ) {
            m_ItemList.append( child );
            insertItemChildren( child );
            child = dynamic_cast<UMLListViewItem *>( child->nextSibling() );
        }
    }
    return true;
}

bool UMLClipboard::checkPasteWidgets( UMLWidgetList & widgetList ) {
    bool retval = true;
    UMLWidget * p = 0;
    UMLWidgetListIt it( widgetList );
    while ( ( p = it.current()) != 0 ) {
        ++it;
        switch( p -> getBaseType() ) {
        case Uml::wt_Note:
            break;

        case Uml::wt_Text:
            {
                FloatingTextWidget *ft = static_cast<FloatingTextWidget*>(p);
                if (ft->getRole() != Uml::tr_Floating) {
                    widgetList.remove(p);
                    delete ft;
                    retval = false;
                }
            }
            break;

        default:
            widgetList.remove(p);
            delete p;
            retval = false;
            break;
        }
    }
    return retval;
}

void UMLClipboard::pasteItemAlreadyExists() {
    UMLView *currentView = UMLApp::app()->getCurrentView();
    KMessageBox::sorry( currentView,
                        i18n("At least one of the items in the clipboard "
                             "could not be pasted because an item of the "
                             "same name already exists.  Any other items "
                             "have been pasted."),
                        i18n("Paste Error") );
}

#include "umlclipboard.moc"

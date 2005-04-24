 /*
  *  copyright (C) 2002-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "umlclipboard.h"
#include "umldrag.h"
#include "idchangelog.h"
#include "../associationwidget.h"
#include "../attribute.h"
#include "../class.h"
#include "../classifier.h"
#include "../floatingtext.h"
#include "../operation.h"
#include "../umldoc.h"
#include "../umllistview.h"
#include "../umllistviewitem.h"
#include "../umlobjectlist.h"
#include "../umlview.h"
#include "../umlwidget.h"
#include "../uml.h"

UMLClipboard::UMLClipboard() {
	m_type = clip1;
}

UMLClipboard::~UMLClipboard() {
}

QMimeSource* UMLClipboard::copy(UMLDoc* Doc, bool fromView/*=false*/) {
	if (!Doc) {
		return 0;
	}
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
		Doc->getCurrentView() -> checkSelections();
		if(!Doc->getCurrentView()->getSelectedWidgets(m_WidgetList)) {
			return 0;
		}
		//if there is no selected widget then there is no copy action
		if(!m_WidgetList.count()) {
			return 0;
		}
		if(!Doc->getCurrentView()->getSelectedAssocs(m_AssociationList)) {
			return 0;
		}
		Doc->getCurrentView()->copyAsImage(png);
	} else { //if the copy action is being performed from the ListView
		if(!listView->getSelectedItems(selectedItems)) {
			return 0;
		}
		//Set What type of copy operation are we performing and
		//also fill m_ViewList with all the selected Diagrams
		setCopyType(selectedItems, Doc);

		//if we are copying a diagram or part of a diagram, select the items
		//on the ListView that correspond to a UseCase, Actor or Concept
		//in the Diagram
		if(m_type == clip2) {
			//Fill the member lists with all the object and stuff to be copied
			//to the clipboard
			selectedItems.clear();
			UMLListViewItem* item = 0;
			UMLViewListIt view_it(m_ViewList);
			//For each selected view select all the Actors, USe Cases and Concepts
			//widgets in the ListView
			UMLObjectList* objects = 0;
			for( ; view_it.current(); ++view_it) {
				objects = Doc -> findView( view_it.current()->getID() ) -> getUMLObjects();
				UMLObjectListIt object_it(*objects);
				for( ; object_it.current(); ++object_it) {
					item = listView->findUMLObject(object_it.current());
					if(item) {
						listView->setSelected(item, true);
					}
				}
				delete objects;
				objects = 0;
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
			data = new UMLDrag(m_ObjectList, m_ItemList);
			break;
		case clip2:
			data = new UMLDrag(m_ObjectList, m_ItemList, m_ViewList);
			break;
		case clip3:
			data = new UMLDrag(m_ItemList);
			break;
		case clip4:
			if(png) {
				data = new UMLDrag(m_ObjectList, m_ItemList, m_WidgetList,
				                    m_AssociationList, *png, Doc -> getCurrentView() -> getType() );
			} else {
				return 0;
			}
			break;
		case clip5:
			data = new UMLDrag(m_ObjectList, m_ItemList, i);
			// The int i is used to differentiate
			// which UMLDrag constructor gets called.
			break;
	}

	return (QMimeSource*)data;
}

bool UMLClipboard::paste(UMLDoc * Doc, QMimeSource* Data) {
	if(!Doc) {
		return false;
	}
	bool result = false;
	Doc->beginPaste();
	switch(UMLDrag::getCodingType(Data)) {
		case 1:
			result = pasteClip1(Doc, Data);
			break;
		case 2:
			result = pasteClip2(Doc, Data);
			break;
		case 3:
			result = pasteClip3(Doc, Data);
			break;
		case 4:
			result = pasteClip4(Doc, Data);
			break;
		case 5:
			result = pasteClip5(Doc, Data);
			break;
		default:
			break;
	}
	Doc->endPaste();
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
				if ( !UMLListView::typeIsClassifierList(type) ) {
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
				if ( !UMLListView::typeIsClassifierList(type) ) {

					m_ItemList.append(item);

					if ( UMLListView::typeIsCanvasWidget(type) ) {
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
				if( UMLListView::typeIsClassifierList(type) ) {
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

void UMLClipboard::setCopyType(UMLListViewItemList& SelectedItems, UMLDoc* Doc) {
	bool withDiagrams = false; //If the selection includes diagrams
	bool withObjects = false; //If the selection includes objects
	bool onlyAttsOps = false; //If the selection only includes Attributes and/or Operations
	UMLListViewItemListIt it(SelectedItems);
	for ( ; it.current(); ++it ) {
		checkItemForCopyType(it.current(), withDiagrams, withObjects, onlyAttsOps, Doc);
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
        bool &OnlyAttsOps, UMLDoc* Doc) {
	if(!Item) {
		return;
	}
	OnlyAttsOps = true;
	UMLView * view = 0;
	UMLListViewItem * child = 0;
	Uml::ListView_Type type = Item->getType();
	if ( UMLListView::typeIsCanvasWidget(type) ) {
			WithObjects = true;
			OnlyAttsOps = false;
	} else if ( UMLListView::typeIsDiagram(type) ) {
			WithDiagrams = true;
			OnlyAttsOps = false;
			view = Doc->findView( Item->getID() );
			m_ViewList.append( view );
	} else if ( UMLListView::typeIsFolder(type) ) {
			OnlyAttsOps = false;
			if(Item->childCount()) {
				child = (UMLListViewItem*)Item->firstChild();
				while(child) {
					checkItemForCopyType(child, WithDiagrams, WithObjects, OnlyAttsOps, Doc);
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

bool UMLClipboard::pasteChildren(UMLListViewItem *parent, IDChangeLog *chgLog, UMLDoc *doc) {
	if (!parent || !doc) {
		kdWarning() << "Paste Children Error, parent or doc missing" << endl;
		return false;
	}
	UMLListView *listView = UMLApp::app()->getListView();
	UMLListViewItem *childItem = static_cast<UMLListViewItem*>(parent->firstChild());
	while (childItem) {
		Uml::IDType oldID = childItem->getID();
		Uml::IDType newID = chgLog->findNewID(oldID);
		UMLListViewItem *shouldNotExist = listView->findItem(newID);
		if (shouldNotExist) {
			kdError() << "UMLClipboard::pasteChildren: new list view item " << ID2STR(newID)
				  << " already exists (internal error)" << endl;
			childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
			continue;
		}
		UMLObject *newObj = doc->findObjectById(newID);
		if (newObj) {
			kdDebug() << "UMLClipboard::pasteChildren: adjusting lvitem(" << ID2STR(oldID)
				  << ") to new UMLObject(" << ID2STR(newID) << ")" << endl;
			childItem->setUMLObject(newObj);
			childItem->setText(newObj->getName());
		} else {
			kdDebug() << "UMLClipboard::pasteChildren: no UMLObject found for lvitem "
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
bool UMLClipboard::pasteClip1(UMLDoc* doc, QMimeSource* data) {
	UMLListViewItemList itemdatalist;
	UMLObjectList objects;
	IDChangeLog* idchanges = doc->getChangeLog();
	if (! UMLDrag::decodeClip1(data, objects, itemdatalist, doc)) {
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
			if(!pasteChildren(itemdata, idchanges, doc)) {
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
bool UMLClipboard::pasteClip2(UMLDoc* doc, QMimeSource* data) {

	UMLListViewItemList itemdatalist;
	UMLObjectList objects;
	objects.setAutoDelete(false);
	UMLViewList		views;
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
			kdDebug()<<"UMLClipboard: error adding umlobject"<<endl;
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
			if(!pasteChildren(item, idchanges, doc)) {
				return false;
			}
		}
		++it;
	}

	return result;
}

/** If clipboard has mime type application/x-uml-clip3,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip3(UMLDoc* doc, QMimeSource* data) {

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
			if(!pasteChildren(item, idchanges, doc)) {
				return false;
			}
		}
		++it;
	}

	return result;
}

/** If clipboard has mime type application/x-uml-clip4,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip4(UMLDoc* doc, QMimeSource* data) {
	if(!doc->getCurrentView()) {
		return false;
	}
	UMLListViewItemList itemdatalist;
	itemdatalist.setAutoDelete(false);

	UMLObjectList objects;
	objects.setAutoDelete(false);


	UMLWidgetList		widgets;
	widgets.setAutoDelete(false);

	AssociationWidgetList	assocs;
	assocs.setAutoDelete(false);

	IDChangeLog* idchanges = 0;

	Uml::Diagram_Type diagramType;

	if( !UMLDrag::decodeClip4(data, objects, itemdatalist, widgets, assocs, diagramType, doc) ) {
		return false;
	}

	if( diagramType != doc->getCurrentView()->getType() ) {
		if( !checkPasteWidgets(widgets) ) {
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
	doc->getCurrentView()->beginPartialWidgetPaste();
	UMLWidget* widget =0;
	UMLWidgetListIt widget_it(widgets);
	while ( (widget=widget_it.current()) != 0 ) {
		++widget_it;

		if (doc->getCurrentView()->findWidget(idchanges->findNewID(widget->getID()))) {
			objectAlreadyExists = true;
		}

		if ( !doc->getCurrentView()->addWidget(widget, true) ) {
			doc->getCurrentView()->endPartialWidgetPaste();
			return false;
		}
	}

	//now paste the associations
	AssociationWidget* assoc;
	AssociationWidgetListIt assoc_it(assocs);
	while ( (assoc=assoc_it.current()) != 0 ) {
		++assoc_it;
		if( !doc->getCurrentView()->addAssociation(assoc, true) ) {
			doc->getCurrentView()->endPartialWidgetPaste();
			return false;
		}
	}

	//Activate all the pasted associations and widgets
	if(!doc->getCurrentView()->activate()) {
		doc->getCurrentView()->endPartialWidgetPaste();
		return false;
	}
	doc->getCurrentView()->endPartialWidgetPaste();

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
			if(!pasteChildren(item, idchanges, doc)) {
				return false;
			}
		}
		++it;
	}

	if (objectAlreadyExists) {
		pasteItemAlreadyExists(doc);
	}
	return true;
}

/** If clipboard has mime type application/x-uml-clip5,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip5(UMLDoc* doc, QMimeSource* data) {
	UMLListView *listView = UMLApp::app()->getListView();
	UMLListViewItem* lvitem = dynamic_cast<UMLListViewItem *>( listView->currentItem() );
	if (!lvitem ||
	    (lvitem->getType() != Uml::lvt_Class && lvitem->getType() != Uml::lvt_Interface)) {
		return false;
	}
	UMLClassifier *parent = dynamic_cast<UMLClassifier *>(lvitem->getUMLObject());
	if (parent == NULL) {
		kdError() << "UMLClipboard::pasteClip5: parent is not a UMLClassifier"
			  << endl;
		return false;
	}
	UMLListViewItemList itemdatalist;
	UMLObjectList objects;
	objects.setAutoDelete(false);
	IDChangeLog* idchanges = 0;
	bool result = UMLDrag::decodeClip5(data, objects, itemdatalist, parent);

	if(!result) {
		return false;
	}

	bool objectAlreadyExists = false;
	UMLObject 	*obj = 0;
	UMLObjectListIt object_it(objects);
	doc->setModified(true);
	idchanges = doc->getChangeLog();

	while ( (obj=object_it.current()) != 0 ) {
		obj->setID(doc->assignNewID(obj->getID()));
		switch(obj->getBaseType()) {
			case Uml::ot_Attribute :
			{
				UMLClass *pClass = (UMLClass *)parent;
				if (pClass->addAttribute(dynamic_cast<UMLAttribute*>(obj), idchanges)) {
//FIXME					doc -> signalChildUMLObjectCreated(obj);
				} else {
					objectAlreadyExists = true;
				}
				break;
			}
			case Uml::ot_Operation :
			{
				UMLClassifier * parent = dynamic_cast<UMLClassifier *>(lvitem -> getUMLObject());
				if (parent -> addOperation(dynamic_cast<UMLOperation*>(obj), idchanges)) {
	//FIXME				doc -> signalChildUMLObjectCreated(obj);
				} else {
					objectAlreadyExists = true;
				}
				break;
			}
			default :
				kdWarning() << "pasteing unknown children type in clip type 5" << endl;
				return false;
		}
		++object_it;
	}

	if (objectAlreadyExists) {
		pasteItemAlreadyExists(doc);
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
	UMLWidget * p = 0;
	UMLWidgetListIt it( widgetList );
	while ( ( p = it.current()) != 0 ) {
		++it;
		switch( p -> getBaseType() ) {
			case Uml::wt_Note:
				break;

			case Uml::wt_Text:
				if( static_cast<FloatingText *>( p )->
				        getRole() != Uml::tr_Floating )
					return false;
				break;

			default:
				return false;
				break;
		}
	}
	return true;
}

void UMLClipboard::pasteItemAlreadyExists(UMLDoc* doc) {
	KMessageBox::sorry( doc->getCurrentView(), i18n("At least one of the items in the clipboard "
							"could not be pasted because an item of the "
							"same name already exists.  Any other items "
							"have been pasted."), i18n("Paste Error") );
}


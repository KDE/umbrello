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

UMLClipboard::UMLClipboard() {
	m_pObjectList = new UMLObjectList;
	m_pObjectList->setAutoDelete( false );
	m_pItemDataList = new UMLListViewItemDataList;
	m_pItemDataList->setAutoDelete( false );
	m_pWidgetList = new UMLWidgetList;
	m_pWidgetList->setAutoDelete( false );
	m_pAssociationList = new AssociationWidgetList;
	m_pAssociationList->setAutoDelete( false );
	m_pViewList = new UMLViewList;
	m_pViewList->setAutoDelete( false );
	m_type = clip1;
}

UMLClipboard::~UMLClipboard() {
    delete m_pObjectList;
    delete m_pItemDataList;
    delete m_pWidgetList;
    delete m_pAssociationList;
    delete m_pViewList;

}

QMimeSource* UMLClipboard::copy(UMLDoc* Doc, bool fromView/*=false*/) {
	if (!Doc) {
		return 0;
	}
	//Clear previous copied data
	m_pAssociationList->clear();
	m_pItemDataList->clear();
	m_pObjectList->clear();
	m_pViewList->clear();

	UMLDrag *data = 0;
	QPixmap* png = 0;

	UMLListView * listView = Doc->listView;
	UMLListViewItemList selectedItems;
	selectedItems.setAutoDelete(false);

	if(fromView) {
		m_type = clip4;
		Doc->getCurrentView() -> checkSelections();
		if(!Doc->getCurrentView()->getSelectedWidgets(*m_pWidgetList)) {
			return 0;
		}
		//if there is no selected widget then there is no copy action
		if(!m_pWidgetList->count()) {
			return 0;
		}
		UMLWidgetListIt widget_it(*m_pWidgetList);
		UMLWidget* widget = widget_it.current();
		UMLObject* object = 0;
		while(widget) {
			++widget_it;
			if ( UMLWidget::widgetHasUMLObject(widget->getBaseType()) ) {
				object = Doc->findUMLObject(widget->getID());
				//if the object is not already on the list
				if(m_pObjectList->find(object) == -1) {
					m_pObjectList->append(object);
				}
			}
			widget = widget_it.current();
		}
		if(!Doc->getCurrentView()->getSelectedAssocs(*m_pAssociationList)) {
			return 0;
		}

		//For each Selected UMLObject get its UMLListViewItemData and children
		UMLObjectListIt object_it(*m_pObjectList);
		object = object_it.current();
		UMLListViewItem* item = 0;
		while(object) {
			++object_it;
			item = listView->findItem(object->getID());
			if(item) {
				m_pItemDataList->append( item->getdata() );
				insertItemDataChildren( item );
			} else {
				return 0;
			}
			object = object_it.current();
		}


		Doc->getCurrentView()->copyAsImage(png);
	} else { //if the copy action is being performed from the ListView
		if(!listView->getSelectedItems(selectedItems)) {
			return 0;
		}
		//Set What type of copy operation are we performing and
		//also fill m_pViewList with all the selected Diagrams
		setCopyType(selectedItems, Doc);

		//if we are copying a diagram or part of a diagram, select the items
		//on the ListView that correspond to a UseCase, Actor or Concept
		//in the Diagram
		if(m_type == clip2) {
			//Fill the member lists with all the object and stuff to be copied
			//to the clipboard
			selectedItems.clear();
			UMLListViewItem* item = 0;
			UMLViewListIt view_it(*m_pViewList);
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
			data = new UMLDrag(*m_pObjectList, *m_pItemDataList);
			break;
		case clip2:
			data = new UMLDrag(*m_pObjectList, *m_pItemDataList, *m_pViewList);
			break;
		case clip3:
			data = new UMLDrag(*m_pItemDataList);
			break;
		case clip4:
			if(png) {
				data = new UMLDrag(*m_pObjectList, *m_pItemDataList, *m_pWidgetList,
				                    *m_pAssociationList, *png, Doc -> getCurrentView() -> getType() );
			} else {
				return 0;
			}
			break;
		case clip5:
			data = new UMLDrag(*m_pObjectList, *m_pItemDataList, i); //The int i is used to differenciate
			//which UMLDrag Constructor gets called
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
					m_pItemDataList->append(item->getdata());
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

					m_pItemDataList->append(item->getdata());

					if ( UMLListView::typeIsCanvasWidget(type) ) {
						m_pObjectList->append(item->getUMLObject());
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
					m_pItemDataList->append(item->getdata());
					m_pObjectList->append(item->getUMLObject());

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
	} else {
		if(withObjects) {
			m_type = clip1;
		} else {
			m_type = clip3;
		}
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
			m_pViewList->append( view );
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

/** Adds the children of a UMLListViewItem to m_pItemDataList */
bool UMLClipboard::insertItemChildren(UMLListViewItem * Item, UMLListViewItemList& SelectedItems) {
	if(Item->childCount()) {
		UMLListViewItem * child = (UMLListViewItem*)Item->firstChild();
		int type;
		while(child) {
			m_pItemDataList->append(child->getdata());
			type = child->getType();
			if(type == Uml::lvt_Actor || type == Uml::lvt_UseCase || type == Uml::lvt_Class) {
				m_pObjectList->append(child->getUMLObject());
			}
			//if the child is selected, remove it from the list of selected items otherwise it will be
			//inserted twice in m_pObjectList
			if(child->isSelected()) {
				SelectedItems.remove(SelectedItems.find(child) );
			}
			insertItemChildren(child, SelectedItems);
			child = (UMLListViewItem*)child->nextSibling();
		}
	}
	return true;
}

bool UMLClipboard::pasteChildren(UMLListViewItem* Parent, UMLListViewItemDataListIt* It,
                                  IDChangeLog& ChangeLog, UMLDoc * Doc) {
	if(!It || !Parent || !Doc) {
		kdWarning()<<"Paste Children Error, maybe: no It, Parent or Doc"<<endl;
		return false;
	}
	UMLListViewItemData* itemdata = It->current();//itemdata gets the parent's info
	//of children to be added
	UMLListViewItem* item = 0;
	int children = itemdata -> getChildren();
	for(int i = 0; i < children; i++) {
		++(*It);
		itemdata = It->current();
		if(!itemdata) {
			kdWarning()<<"PasteChildren called with no itemData"<<endl;
			return false; //Error this function gets called only if the parent Item
			//has children and that means It->current() shouldn't be null
		}
		item = Doc->listView->createItem(*itemdata, ChangeLog, Parent);
		int count = itemdata -> getChildren();
		if( item && count ) {
			kdDebug()<<count<<endl;
			if(!pasteChildren(item, It, ChangeLog, Doc)) {
				return false;
			}
		}
	}


	return true;
}
/** Cleans the list of associations taking out the ones that point to an object not in
m_pObjectList */
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
	UMLListViewItemDataList itemdatalist;
	UMLObjectList objects;

	objects.setAutoDelete(false);
	IDChangeLog* idchanges = 0;
	bool result = UMLDrag::decodeClip1(data, objects, itemdatalist, doc);
	if(!result) {
		return false;
	}
	UMLObject *obj = 0;
	UMLObjectListIt object_it(objects);
	while ( (obj=object_it.current()) != 0 ) {
		++object_it;
		if(!doc->addUMLObjectPaste(obj)) {
			return false;

		}
		idchanges = doc->getChangeLog();
		if(!idchanges) {
			return false;
		}
	}
	bool objectAlreadyExists = false;
	UMLListViewItem* item = 0;
	UMLListViewItemData* itemdata = 0;
	UMLListViewItemDataListIt it(itemdatalist);
	while ( (itemdata=it.current()) != 0 ) {
		if ( (item = doc->listView->findItem(idchanges->findNewID(itemdata->getID()))) ) {
			objectAlreadyExists = true;
		} else {
			item = doc->listView->createItem(*itemdata, *idchanges);
		}


		if(!item) {
			return false;
		}
		if(itemdata -> getChildren()) {
			if(!pasteChildren(item, &it, *idchanges, doc)) {
				return false;
			}

		}
		++it;
	}

	if (objectAlreadyExists) {
		pasteItemAlreadyExists(doc);
	}
	return result;
}

/** If clipboard has mime type application/x-uml-clip2,
Pastes the data from the clipboard into the current Doc */
bool UMLClipboard::pasteClip2(UMLDoc* doc, QMimeSource* data) {

	UMLListViewItemDataList itemdatalist;
	UMLObjectList objects;
	objects.setAutoDelete(false);
	UMLViewList		views;
	IDChangeLog* idchanges = 0;

	bool result = UMLDrag::decodeClip2(data, objects, itemdatalist, views, doc);
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
		if(!doc->addUMLObjectPaste(obj)) {
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

	UMLListViewItem* item = 0;
	UMLListViewItemData* itemdata = 0;
	UMLListViewItemDataListIt it(itemdatalist);
	while ( (itemdata=it.current()) != 0 ) {
		item = doc->listView->createItem(*itemdata, *idchanges);
		if(!item) {
			return false;
		}
		if(itemdata -> getChildren()) {
			if(!pasteChildren(item, &it, *idchanges, doc)) {
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

	UMLListViewItemDataList itemdatalist;
	UMLListViewItem* item = 0;
	UMLListViewItemData* itemdata = 0;
	IDChangeLog* idchanges = doc->getChangeLog();

	if(!idchanges) {
		return false;
	}

	bool result = UMLDrag::decodeClip3(data, itemdatalist);
	if(!result) {
		return false;
	}
	UMLListViewItemDataListIt it(itemdatalist);
	while ( (itemdata=it.current()) != 0 ) {
		item = doc->listView->createItem(*itemdata, *idchanges);
		if(itemdata -> getChildren()) {
			if(!pasteChildren(item, &it, *idchanges, doc)) {
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
	UMLListViewItemDataList itemdatalist;
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
		if(!doc->addUMLObjectPaste(obj)) {
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

		if ( !doc->getCurrentView()->addWidget(widget) ) {
			doc->getCurrentView()->endPartialWidgetPaste();
			return false;
		}
	}

	//now paste the associations
	AssociationWidget* assoc;
	AssociationWidgetListIt assoc_it(assocs);
	while ( (assoc=assoc_it.current()) != 0 ) {
		++assoc_it;
		if( !doc->getCurrentView()->addAssociation(assoc) ) {
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

	UMLListViewItem* item = 0;
	UMLListViewItemData* itemdata = 0;
	UMLListViewItemDataListIt it(itemdatalist);
	while ( (itemdata=it.current()) != 0 ) {
		item = doc->listView->createItem(*itemdata, *idchanges);
		if(!item) {
			return false;
		}
		if(itemdata -> getChildren()) {
			if(!pasteChildren(item, &it, *idchanges, doc)) {
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
	UMLListViewItem* lvitem = dynamic_cast<UMLListViewItem *>( doc->listView->currentItem() );
	if(!lvitem || (lvitem->getType() != Uml::lvt_Class)) {
		return false;
	}
	UMLListViewItemDataList itemdatalist;
	UMLObjectList objects;
	objects.setAutoDelete(false);
	IDChangeLog* idchanges = 0;
	bool result = UMLDrag::decodeClip5(data, objects, itemdatalist, doc);
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
				UMLClass * parent = dynamic_cast<UMLClass *>(lvitem -> getUMLObject());
				if (parent -> addAttribute(dynamic_cast<UMLAttribute*>(obj), idchanges)) {
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

bool UMLClipboard::insertItemDataChildren( UMLListViewItem * item ) {
	if( item -> childCount() ) {
		UMLListViewItem * child =dynamic_cast<UMLListViewItem *>( item -> firstChild() );
		while( child ) {
			m_pItemDataList -> append( child->getdata() );
			insertItemDataChildren( child );
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
				if( dynamic_cast<FloatingText *>( p )->
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


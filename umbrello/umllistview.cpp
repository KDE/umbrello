
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "actor.h"
#include "classimport.h"
#include "class.h"
#include "package.h"
#include "component.h"
#include "node.h"
#include "artifact.h"
#include "interface.h"
#include "docwindow.h"
#include "listpopupmenu.h"
#include "operation.h"
#include "attribute.h"
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitemlist.h"
#include "umllistviewitem.h"
#include "umlview.h"
#include "usecase.h"
#include "clipboard/idchangelog.h"
#include "clipboard/umldrag.h"
#include "dialogs/classpropdlg.h"
#include "dialogs/umlattributedialog.h"
#include "dialogs/umloperationdialog.h"
#include <kiconloader.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include "diagram/diagram.h"

//using Umbrello::Diagram;

UMLListView::UMLListView(QWidget *parent,const char *name) : KListView(parent,name), m_pMenu(0) {
	loadPixmaps();

	//setup list view
	setBackgroundColor(white);
	setAcceptDrops(true);
	setDropVisualizer(false);
	setItemsMovable(true);
	setItemsRenameable( true );
	setSelectionModeExt(FileManager);
	setShowToolTips( true );
	setTooltipColumn( 0 );
	setFocusPolicy(QWidget::StrongFocus);
	setDragEnabled(TRUE);
	setColumnWidthMode( 0, Manual );
	setDefaultRenameAction( Accept );
	setResizeMode( LastColumn );
	//add columns and initial items
	addColumn(i18n("UML Diagrams"));

	rv =  new UMLListViewItem(this, i18n("Views"), Uml::lvt_View);
	ucv = new UMLListViewItem(rv, i18n("Use Case View"), Uml::lvt_UseCase_View);
	lv = new UMLListViewItem(rv, i18n("Logical View"), Uml::lvt_Logical_View);
	componentView = new UMLListViewItem(rv, i18n("Component View"), Uml::lvt_Component_View);
	deploymentView = new UMLListViewItem(rv, i18n("Deployment View"), Uml::lvt_Deployment_View);
//Uncomment for using Luis diagram display code
//	diagramFolder = new UMLListViewItem(rv,i18n("Diagrams"),Uml::lvt_Diagrams);


	init();

	//setup slots/signals
	connect(this, SIGNAL(dropped(QDropEvent *, QListViewItem *, QListViewItem *)),
		this, SLOT(slotDropped(QDropEvent *, QListViewItem *, QListViewItem *)));
	connect( this, SIGNAL( collapsed( QListViewItem * ) ),
		 this, SLOT( slotCollapsed( QListViewItem * ) ) );
	connect( this, SIGNAL( expanded( QListViewItem * ) ), this, SLOT( slotExpanded( QListViewItem * ) ) );
	connect( UMLApp::app(), SIGNAL( sigCutSuccessful() ), this, SLOT( slotCutSuccessful() ) );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListView::~UMLListView() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::contentsMousePressEvent(QMouseEvent *me) {
	if( m_doc -> getCurrentView() )
		m_doc -> getCurrentView() -> clearSelected();
	if( me -> state() != ShiftButton )
		clearSelection();
	//UMLListViewItem *item = (UMLListViewItem *)currentItem();
	QPoint pt = ((UMLListView*)this) -> contentsToViewport( me -> pos() );
	UMLListViewItem * item = (UMLListViewItem*)itemAt(pt);


	if(  me -> button() != RightButton && me -> button() != LeftButton  ) {
		m_doc -> getDocWindow() -> updateDocumentation( true );
		return;
	}

	Uml::ListView_Type type;
	type = item ? item->getType() : lvt_Unknown;
	if (me->button() == LeftButton) {
		switch( type ) {
			//If is necesary activate the view
		case Uml::lvt_UseCase_Diagram:
		case Uml::lvt_Class_Diagram:
		case Uml::lvt_Collaboration_Diagram:
		case Uml::lvt_Sequence_Diagram:
		case Uml::lvt_State_Diagram:
		case Uml::lvt_Activity_Diagram:
		case Uml::lvt_Component_Diagram:
		case Uml::lvt_Deployment_Diagram:
			if( ! m_doc->activateView( item->getID()))
				KMessageBox::error( kapp -> mainWidget(), i18n("Could not activate the diagram."), i18n("Diagram Load Error"));
			else
				m_doc -> changeCurrentView(item->getID());
			emit diagramSelected( item->getID());
			m_doc -> getDocWindow() -> showDocumentation( m_doc -> findView( item -> getID() ), false );
			break;

		case Uml::lvt_UseCase:
		case Uml::lvt_Class:
		case Uml::lvt_Package:
		case Uml::lvt_Component:
		case Uml::lvt_Node:
		case Uml::lvt_Interface:
		case Uml::lvt_Actor:
		case Uml::lvt_Attribute:
		case Uml::lvt_Operation:
		case Uml::lvt_Template:
			m_doc -> getDocWindow() -> showDocumentation( item -> getUMLObject(), false );
			break;

		default:
			m_doc -> getDocWindow() -> updateDocumentation( true );
			break;
		}
	}//end switch
	if(me->button() == RightButton) {
		//setSelected( item, true);
		if(m_pMenu != 0) {
			m_pMenu->hide();
			disconnect(m_pMenu, SIGNAL(activated(int)), this, SLOT(popupMenuSel(int)));
			delete m_pMenu;
			m_pMenu = 0;
		}

		m_pMenu = new ListPopupMenu(this, type);
		m_pMenu->popup(me->globalPos());
		connect(m_pMenu, SIGNAL(activated(int)), this, SLOT(popupMenuSel(int)));
	}//end if right button

	/*
	  We don't need to do anything for Left button.
	  But if we do in future, just put and else statement in here
	*/
	this->KListView::contentsMousePressEvent(me);


}

void UMLListView::contentsMouseReleaseEvent(QMouseEvent *me) {
	this->KListView::contentsMouseReleaseEvent(me);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::popupMenuSel(int sel) {
	UMLListViewItem * temp = (UMLListViewItem*)currentItem();
	if( !temp ) {
		kdDebug() << "popupMenuSel invoked without currently selectedItem" << endl;
		return;
	}
	int id = temp -> getID();
	UMLObject * object = temp -> getUMLObject();
	Uml::ListView_Type lvtType = temp -> getType();
	Uml::UMLObject_Type umlType = Uml::ot_UMLObject;
	QString name;

	switch(sel) {
	case ListPopupMenu::mt_Activity_Diagram:
		m_doc->createDiagram(dt_Activity);
		break;

	case ListPopupMenu::mt_UseCase_Diagram:
		m_doc->createDiagram(dt_UseCase);
		break;

	case ListPopupMenu::mt_Sequence_Diagram:
		m_doc->createDiagram(dt_Sequence);
		break;

	case ListPopupMenu::mt_Collaboration_Diagram:
		m_doc->createDiagram(dt_Collaboration);
		break;

	case ListPopupMenu::mt_Class_Diagram:
		m_doc->createDiagram(dt_Class);
		break;

	case ListPopupMenu::mt_State_Diagram:
		m_doc->createDiagram(dt_State);
		break;

	case ListPopupMenu::mt_Component_Diagram:
		m_doc->createDiagram(dt_Component);
		break;

	case ListPopupMenu::mt_Deployment_Diagram:
		m_doc->createDiagram(dt_Deployment);
		break;

	case ListPopupMenu::mt_Class:
		addNewItem( temp, Uml::lvt_Class );
		break;

	case ListPopupMenu::mt_Package:
		addNewItem(temp, Uml::lvt_Package);
		break;

	case ListPopupMenu::mt_Component:
		addNewItem(temp, Uml::lvt_Component);
		break;

	case ListPopupMenu::mt_Node:
		addNewItem(temp, Uml::lvt_Node);
		break;

	case ListPopupMenu::mt_Artifact:
		addNewItem(temp, Uml::lvt_Artifact);
		break;

	case ListPopupMenu::mt_Interface:
		addNewItem(temp, Uml::lvt_Interface);
		break;

	case ListPopupMenu::mt_Actor:
		addNewItem( temp, Uml::lvt_Actor );
		break;

	case ListPopupMenu::mt_UseCase:
		addNewItem( temp, Uml::lvt_UseCase );
		break;

	case ListPopupMenu::mt_Attribute:
		addNewItem( temp, Uml::lvt_Attribute );
		break;

	case ListPopupMenu::mt_Operation:
		addNewItem( temp, Uml::lvt_Operation );
		break;

	case ListPopupMenu::mt_Import_Classes:
		((ClassImport*)m_doc)->importCPP(
			KFileDialog::getOpenFileNames(":import-classes",
						      i18n("*.h|Header Files (*.h)\n*|All Files"),
						      this, i18n("Select Classes to Import") ));
	case ListPopupMenu::mt_Expand_All:
		expandAll(temp);
		break;
	case ListPopupMenu::mt_Collapse_All:
		collapseAll(temp);
		break;

	case ListPopupMenu::mt_Export_Image:
		m_doc->getCurrentView()->exportImage();
		break;

	case ListPopupMenu::mt_Rename:
		temp-> startRename(0);
		break;

	case ListPopupMenu::mt_Delete:
		if ( typeIsDiagram(lvtType) ) {
			m_doc->removeDiagram(id);
		} else if( typeIsFolder(lvtType) ) {
			if ( temp->firstChild() ) {
				KMessageBox::error(kapp->mainWidget(), i18n("The folder must be emptied before it can be deleted."), i18n("Folder Not Empty"));
				return;
			}
			delete temp;
		} else {
			m_doc->removeUMLObject(object);
		}
		return;
		break;

	case ListPopupMenu::mt_Properties:
		/* first check if we are on a diagram */
		if( typeIsDiagram(lvtType) ) {
			UMLView * pView = m_doc->findView( temp->getID() );
			if( !pView ) {
				return;
			}
			m_doc->getDocWindow()->updateDocumentation(false);
			pView->showPropDialog();
			m_doc->getDocWindow()->showDocumentation(pView, true);
			temp->cancelRename(0);
			return;
		}

		/* ok, we are on another object, so find out on which one */
		umlType = object->getBaseType();

		if ( typeIsCanvasWidget(lvtType) ) {
			m_doc->showProperties(object, ClassPropDlg::page_gen);
		} else if(umlType == Uml::ot_Attribute) {
			// show the attribute dialogue
			UMLAttribute* selectedAttribute = (UMLAttribute*)object;
			UMLAttributeDialog dialogue( this, selectedAttribute );
			dialogue.exec();
		} else if(umlType == Uml::ot_Operation) {
			// show the operation dialogue
			UMLOperation* selectedOperation = (UMLOperation*)object;
			UMLOperationDialog dialogue( this, selectedOperation );
			dialogue.exec();
		} else {
			kdWarning() << "calling properties on unknown type" << endl;
		}
		temp -> cancelRename( 0 );
		break;

	case ListPopupMenu::mt_Logical_Folder:
		addNewItem( temp, Uml::lvt_Logical_Folder );
		m_doc -> setModified( true );
		break;

	case ListPopupMenu::mt_UseCase_Folder:
		addNewItem( temp, Uml::lvt_UseCase_Folder );
		m_doc -> setModified( true );
		break;

	case ListPopupMenu::mt_Component_Folder:
		addNewItem(temp, Uml::lvt_Component_Folder);
		m_doc->setModified(true);
		break;

	case ListPopupMenu::mt_Deployment_Folder:
		addNewItem(temp, Uml::lvt_Deployment_Folder);
		m_doc->setModified(true);
		break;

	case ListPopupMenu::mt_Cut:
		m_bStartedCut = true;
		m_doc -> editCut();
		break;

	case ListPopupMenu::mt_Copy:
		m_doc -> editCopy();
		break;

	case ListPopupMenu::mt_Paste:
		m_doc -> editPaste();
		break;

	default:
		return;//must be something we don't want to do
		break;
	}//end switch
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::diagramCreated(Diagram* /* d */) {
//Uncomment for using Luis diagram display code
//	new UMLListViewItem( diagramFolder, d->name(),
//			     convert_DT_LVT((Uml::Diagram_Type)d->diagramType()), d->getID() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::slotDiagramCreated( int id ) {
	if( loading )
		return;
	UMLListViewItem * temp = 0, *p = 0;
	UMLView *v = m_doc -> findView( id );
	if( !v )
		return;
	//See if we wanted to create diagram in folder
	UMLListViewItem * current = (UMLListViewItem *) currentItem();
	if ( current && typeIsFolder(current->getType()) ) {
		p = current;
	} else if (v->getType() == Uml::dt_UseCase) {
		p = ucv;
	} else if (v->getType() == Uml::dt_Component) {
		p = componentView;
	} else if (v->getType() == Uml::dt_Deployment) {
		p = deploymentView;
	} else {
		p = lv;
	}
	temp = new UMLListViewItem( p, v->getName(), convert_DT_LVT( v->getType() ),  id );
	setSelected( temp, true );
	m_doc -> getDocWindow() -> showDocumentation( v , false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::slotObjectCreated(UMLObject* object) {
	if (loading) {
		return;
	}
	UMLListViewItem* newItem = 0;
	UMLListViewItem* parentItem = 0;
	UMLListViewItem* current = (UMLListViewItem*) currentItem();
	connectNewObjectsSlots(object);
	Uml::UMLObject_Type type = object->getBaseType();
	switch( type )
	{
	case Uml::ot_Attribute:
	case Uml::ot_Operation:
	case Uml::ot_Template:
	case Uml::ot_Association:
	case Uml::ot_Stereotype:
		//this will be handled by childObjectAdded
		return;
		break;
	case Uml::ot_Class:
		if ( current && current->getType() == Uml::lvt_Logical_Folder ) {
			parentItem = current;
		} else {
			parentItem = lv;
		}
		break;
	case Uml::ot_Interface:
		if ( current && current->getType() == Uml::lvt_Logical_Folder ) {
			parentItem = current;
		} else {
			parentItem = lv;
		}
		break;
	case Uml::ot_Package:
		if ( current && current->getType() == Uml::lvt_Logical_Folder ) {
			parentItem = current;
		} else {
			parentItem = lv;
		}
		break;
	case Uml::ot_Actor:
	case Uml::ot_UseCase:
		if( current && current->getType() == Uml::lvt_UseCase_Folder )
			parentItem = current;
		else
			parentItem = ucv;
		break;
	case Uml::ot_Component:
	case Uml::ot_Artifact:
		if( current && current->getType() == Uml::lvt_Component_Folder )
			parentItem = current;
		else
			parentItem = componentView;
		break;
	case Uml::ot_Node:
		parentItem = deploymentView;
		break;
	default:
		kdWarning() << "UMLListView: no appropriate parent found for " << object->getName()
			    << ", using default" << endl;
		parentItem = ucv;
	}

	newItem = new UMLListViewItem(parentItem, object->getName(), convert_OT_LVT(type), object);
	ensureItemVisible(newItem);
	newItem->setOpen(true);
	clearSelection();
	setSelected(newItem, true);
	m_doc->getDocWindow()->showDocumentation(object, false);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::connectNewObjectsSlots(UMLObject* object) {
	Uml::UMLObject_Type type = object->getBaseType();
	switch( type )
	{
	case Uml::ot_Class:
		connect(object,SIGNAL(attributeAdded(UMLObject*)),
			this,SLOT(childObjectAdded(UMLObject*)));
		connect(object,SIGNAL(attributeRemoved(UMLObject*)),
			this,SLOT(childObjectRemoved(UMLObject*)));
		connect(object,SIGNAL(operationAdded(UMLObject*)),
			this,SLOT(childObjectAdded(UMLObject*)));
		connect(object,SIGNAL(operationRemoved(UMLObject*)),
			this,SLOT(childObjectRemoved(UMLObject*)));
		connect(object,SIGNAL(templateAdded(UMLObject*)),
			this,SLOT(childObjectAdded(UMLObject*)));
		connect(object,SIGNAL(templateRemoved(UMLObject*)),
			this,SLOT(childObjectRemoved(UMLObject*)));
		connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
		break;
	case Uml::ot_Interface:
		connect(object,SIGNAL(operationAdded(UMLObject*)),
			this,SLOT(childObjectAdded(UMLObject*)));
		connect(object,SIGNAL(operationRemoved(UMLObject*)),
			this,SLOT(childObjectRemoved(UMLObject*)));
		connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
		break;
	case Uml::ot_Attribute:
	case Uml::ot_Operation:
	case Uml::ot_Template:
	case Uml::ot_Package:
	case Uml::ot_Actor:
	case Uml::ot_UseCase:
	case Uml::ot_Component:
	case Uml::ot_Artifact:
	case Uml::ot_Node:
		connect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
		break;
	case Uml::ot_UMLObject:
	case Uml::ot_Association:
	case Uml::ot_Stereotype:
		break;
	default:
		kdWarning() << "unknown type in connectNewObjectsSlots" << endl;
		break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::slotObjectChanged() {
	if (loading) { //needed for class wizard
		return;
	}
	UMLObject* obj = const_cast<UMLObject*>( dynamic_cast<const UMLObject*>(sender()) );
	UMLListViewItem* item;
	item = findUMLObject(obj);
	if(item) {
		item->updateObject();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::childObjectAdded(UMLObject* obj) {
	if (obj->getBaseType() == ot_Stereotype) {
		return;
	}
	if (!m_bCreatingChildObject) {
		UMLObject *parent = const_cast<UMLObject*>(dynamic_cast<const UMLObject*>(sender()));

		UMLListViewItem *parentItem = findUMLObject(parent);
		UMLListViewItem *newItem = new UMLListViewItem(parentItem, obj->getName(), convert_OT_LVT(obj->getBaseType()), obj);

		ensureItemVisible(newItem);
		clearSelection();
		setSelected(newItem, true);
	}
	connectNewObjectsSlots(obj);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::childObjectRemoved(UMLObject* obj) {
	UMLObject *parent = const_cast<UMLObject*>(dynamic_cast<const UMLObject*>(sender()));
	UMLListViewItem *item(0);
	UMLListViewItem *parentItem = findUMLObject(parent);
	for( item = static_cast<UMLListViewItem*>(parentItem->firstChild());
	     item;
	     item = static_cast<UMLListViewItem*>(item->nextSibling()) )
	{
	     	if(item->getUMLObject() == obj)
		{
			delete item;
			return;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::slotDiagramRenamed(int id) {
	UMLListViewItem* temp;
	UMLView* v = m_doc->findView(id);
	temp = findView(v);
	temp->setText( v->getName() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::setDocument(UMLDoc *d) {
	if( m_doc && m_doc != d)
	{
		//disconnect signals from old doc and reset view
	}
	m_doc = d;


	connect(m_doc, SIGNAL(sigDiagramCreated(int)), this, SLOT(slotDiagramCreated(int)));
	connect(m_doc, SIGNAL(diagramCreated(Umbrello::Diagram*)), this, SLOT( diagramCreated(Umbrello::Diagram*)));
	connect(m_doc, SIGNAL(sigDiagramRemoved(int)), this, SLOT(slotDiagramRemoved(int)));
	connect(m_doc, SIGNAL(sigDiagramRenamed(int)), this, SLOT(slotDiagramRenamed(int)));

	connect(m_doc, SIGNAL(sigObjectCreated(UMLObject *)), this, SLOT(slotObjectCreated(UMLObject *)));
	connect(m_doc, SIGNAL(sigObjectRemoved(UMLObject *)), this, SLOT(slotObjectRemoved(UMLObject *)));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::slotObjectRemoved(UMLObject* object) {
	if (loading) { //needed for class wizard
		return;
	}
	disconnect(object,SIGNAL(modified()),this,SLOT(slotObjectChanged()));
	UMLListViewItem* item = findItem(object->getID());
	delete item;
	m_doc->getDocWindow()->updateDocumentation(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::slotDiagramRemoved(int id) {
	UMLListViewItem* item = findItem(id);
	delete item;
	m_doc->getDocWindow()->updateDocumentation(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QDragObject* UMLListView::dragObject() {
	UMLListViewItemList selecteditems;
	getSelectedItems(selecteditems);
	selecteditems.setAutoDelete( FALSE );
	UMLListViewItemListIt it(selecteditems);
	UMLListViewItem * item = 0;
	UMLListViewItemList  list;
	list.setAutoDelete( FALSE );
	Uml::ListView_Type type;
	while((item=it.current()) != 0) {
		++it;
		type = item->getType();
		if ( !typeIsCanvasWidget(type) && !typeIsDiagram(type) && !typeIsFolder(type) ) {
			return 0;
		}
		list.append(item);
	}
	UMLDrag *t = new UMLDrag(list, this);

	return t;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::startDrag() {
	QTextDrag * o=(QTextDrag*)dragObject();
	if(o)
		o->dragCopy();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem * UMLListView::findUMLObjectInFolder(UMLListViewItem* folder, UMLObject* obj) {
	UMLListViewItem *item = static_cast<UMLListViewItem *>(folder->firstChild());
	while(item)
	{
		switch(item->getType())
		{
		case Uml::lvt_Actor :
		case Uml::lvt_UseCase :
		case Uml::lvt_Class :
		case Uml::lvt_Package :
		case Uml::lvt_Component :
		case Uml::lvt_Node :
		case Uml::lvt_Artifact :
		case Uml::lvt_Interface :
			if(item->getUMLObject() == obj)
				return item;
			break;
		case Uml::lvt_Logical_Folder :
		case Uml::lvt_UseCase_Folder :
		case Uml::lvt_Component_Folder :
		case Uml::lvt_Deployment_Folder :
		{
			UMLListViewItem *temp = findUMLObjectInFolder(item, obj);
			if (temp)
				return temp;
		}
		default:
			break;
		}
		item = static_cast<UMLListViewItem *>(item->nextSibling());
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem * UMLListView::findUMLObject(UMLObject *p) const {
	UMLListViewItem *item = static_cast<UMLListViewItem*>(firstChild());
	while( item && item->getUMLObject() != p )
	{
		item = static_cast<UMLListViewItem*>(item->itemBelow());
	}
	return item;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem* UMLListView::findView(UMLView* v) {
	if (!v) {
		kdWarning() << "returning 0 from UMLListView::findView()" << endl;
		return 0;
	}
	UMLListViewItem* item;
	Uml::Diagram_Type dType = v->getType();
	Uml::ListView_Type type = convert_DT_LVT( dType );
	int id = v->getID();
	if (dType == Uml::dt_UseCase) {
		item = ucv;
	} else if (dType == Uml::dt_Component) {
		item = componentView;
	} else if (dType == Uml::dt_Deployment) {
		item = deploymentView;
	} else {
		item = lv;
	}

	UMLListViewItem* searchStartItem = (UMLListViewItem *)item->firstChild();

	UMLListViewItem* foundItem = recursiveSearchForView(searchStartItem, type, id);

	if (!foundItem) {
		kdWarning() << "returning 0 at UMLListView::findView" << endl;
	}
	return foundItem;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem* UMLListView::recursiveSearchForView(UMLListViewItem* listViewItem, ListView_Type type, int id) {
	while (listViewItem) {
		if ( typeIsFolder(listViewItem->getType()) ) {
			UMLListViewItem* child = (UMLListViewItem *)listViewItem->firstChild();
			UMLListViewItem* resultListViewItem = recursiveSearchForView(child, type, id);
			if (resultListViewItem) {
				return resultListViewItem;
			}
		} else {
			if(listViewItem->getType() == type && listViewItem->getID() == id) {
				return listViewItem;
			}
		}
		listViewItem = (UMLListViewItem*)listViewItem->nextSibling();
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem* UMLListView::findItem(int id) {
	UMLListViewItem *temp;
	QListViewItemIterator it(this);
	for( ; (temp = (UMLListViewItem*)it.current()); ++it )
		if(temp->getID() == id)
			return temp;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

//
// This method is called more than once during an instance's lifetime (by UMLDoc)!
// So we must not allocate any memory before freeing the previously allocated one
// or do connect()s.
//
void UMLListView::init() {
	deleteChildrenOf( ucv );
	deleteChildrenOf( lv );
	deleteChildrenOf( componentView );
	deleteChildrenOf( deploymentView );
//Uncomment for using Luis diagram display code
//	deleteChildrenOf( diagramFolder );

	rv->setOpen(true);
	ucv->setOpen(true);
	lv->setOpen(true);
	componentView->setOpen(true);
	deploymentView->setOpen(true);

	//setup misc.
	delete m_pMenu;
	m_pMenu = 0;
	m_bStartedCut = false;
	loading = false;
	m_bIgnoreCancelRename = true;
	m_bCreatingChildObject = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::setView(UMLView * v) {
	if(!v)
		return;
	UMLListViewItem * temp = findView(v);
	if(temp)
		setSelected(temp, true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::setLoading(bool state) {
	loading = state;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListView::contentsMouseDoubleClickEvent(QMouseEvent * me) {
	UMLListViewItem * item = static_cast<UMLListViewItem *>( currentItem() );
	if( !item || me -> button() != LeftButton )
		return;
	//see if on view
	Uml::ListView_Type lvType = item -> getType();
	if( typeIsDiagram(lvType) ) {
		UMLView * pView = m_doc -> findView( item -> getID() );
		if( !pView )
			return;
		m_doc -> getDocWindow() -> updateDocumentation( false );
		pView -> showPropDialog();
		m_doc -> getDocWindow() -> showDocumentation( pView, true );
		item -> cancelRename( 0 );
		return;
	}
	//else see if an object
	UMLObject * object = item -> getUMLObject();
	//continue only if we are on a UMLObject
	if(!object)
		return;


	Uml::UMLObject_Type type = object -> getBaseType();
	int page = ClassPropDlg::page_gen;
	if(type == Uml::ot_Attribute || type == Uml::ot_Operation)
		object = (UMLObject *)object -> parent();
	//set what page to show
	if(type == Uml::ot_Attribute)
		page = ClassPropDlg::page_att;
	else if(type == Uml::ot_Operation)
		page = ClassPropDlg::page_op;

	if(object)
		m_doc -> showProperties(object, page);
	item -> cancelRename( 0 );//double click can cause it to go into rename mode.
}


bool UMLListView::acceptDrag(QDropEvent* event) const {
	QPoint mousePoint = ((UMLListView*)this)->contentsToViewport( event->pos() );

	UMLListViewItem* item = (UMLListViewItem*)itemAt(mousePoint);
	if(!item) {
		return false;
	}
	((QListView*)this)->setCurrentItem( (QListViewItem*)item );
	UMLListViewItemList list;

	bool status = UMLDrag::decodeClip3(event, list);
	if(!status) {
		return false;
	}

	UMLListViewItemListIt it(list);
	UMLListViewItem* data = 0;
	int itemType = item->getType();
	bool accept = true;
	while(accept && ((data = it.current()) != 0)) {
		++it;
		Uml::ListView_Type lvType = data->getType();
		if((lvType == Uml::lvt_Class || lvType == Uml::lvt_Package
		    || lvType == Uml::lvt_Interface || lvType == Uml::lvt_Class_Diagram
		    || lvType == Uml::lvt_Collaboration_Diagram
		    || lvType == Uml::lvt_State_Diagram
		    || lvType == Uml::lvt_Activity_Diagram
		    || lvType == Uml::lvt_Sequence_Diagram)
		   && (itemType == Uml::lvt_Logical_Folder || itemType == Uml::lvt_Logical_View) ) {
			continue;
		}
		if( (lvType == Uml::lvt_Actor || lvType == Uml::lvt_UseCase
		     || lvType == Uml::lvt_UseCase_Diagram)
		    && (itemType == Uml::lvt_UseCase_Folder || itemType == Uml::lvt_UseCase_View) ) {
			continue;
		}
		if( (lvType == Uml::lvt_Component || lvType == Uml::lvt_Artifact
		     || lvType == Uml::lvt_Component_Diagram)
		    && (itemType == Uml::lvt_Component_Folder) ) {
			continue;
		}
		if( (lvType == Uml::lvt_Node ||
		     lvType == Uml::lvt_Deployment_Diagram)
		    && itemType == Uml::lvt_Deployment_Folder ) {
			continue;
		}
		accept = false;
	}

	return accept;
}

void UMLListView::slotDropped(QDropEvent* de, QListViewItem* parent, QListViewItem* item) {
	parent = 0;//done just to stop compiler warning
	if(!item) {
		return;
	}
	UMLListViewItemList list;
	bool status = UMLDrag::decodeClip3(de, list);

	if(!status) {
		return;
	}
	Uml::ListView_Type itemType = ((UMLListViewItem*)item) -> getType();
	UMLListViewItemListIt it(list);
	UMLListViewItem* data = 0;
	while((data =  it.current()) != 0) {
		++it;
		UMLListViewItem * move;
		Uml::ListView_Type lvType = data->getType();
		if( typeIsDiagram(lvType) ) {
			UMLView * v = m_doc -> findView(data->getID());
			move = findView(v);
		} else {
			UMLObject * o = m_doc -> findUMLObject(data->getID());
			move = findUMLObject(o);
		}
		if(!move)
			continue;
		//make sure trying to place in correct location

		if( (lvType == Uml::lvt_UseCase_Folder
		     || lvType == Uml::lvt_Actor
		     || lvType == Uml::lvt_UseCase
		     || lvType == Uml::lvt_UseCase_Diagram)
		    && (itemType == Uml::lvt_UseCase_Folder || itemType == Uml::lvt_UseCase_View) ) {
			moveItem(move, item, item);
		}
		if( (lvType == Uml::lvt_Component_Folder
		     || lvType == Uml::lvt_Component
		     || lvType == Uml::lvt_Artifact
		     || lvType == Uml::lvt_Component_Diagram)
		    && (itemType == Uml::lvt_Component_Folder || itemType == Uml::lvt_Component_View) ) {
			moveItem(move, item, item);
		}
		if( (lvType == Uml::lvt_Deployment_Folder
		     || lvType == Uml::lvt_Node
		     || lvType == Uml::lvt_Deployment_Diagram)
		    && (itemType == Uml::lvt_Deployment_Folder || itemType == Uml::lvt_Deployment_View) ) {
			moveItem(move, item, item);
		}
		if( ((lvType >= Uml::lvt_Collaboration_Diagram
		      && lvType <= Uml::lvt_Sequence_Diagram)
		     || lvType == Uml::lvt_Class
		     || lvType == Uml::lvt_Package
		     || lvType == Uml::lvt_Interface)
		    && (itemType == Uml::lvt_Logical_Folder || itemType == Uml::lvt_Logical_View)) {
			moveItem(move, item, item);
		}
	}
}

bool UMLListView::getSelectedItems(UMLListViewItemList &ItemList) {
	ItemList.setAutoDelete( FALSE );
	QListViewItemIterator it(this);
	// iterate through all items of the list view
	for ( ; it.current(); ++it ) {
		if ( it.current()->isSelected() ) {
			ItemList.append((UMLListViewItem*)it.current());
		}
	}

	return true;
}

/** Creates a new UMLListViewItem from a UMLListViewItem,
    if parent is null the ListView Decides who is going to be
    the parent */
UMLListViewItem* UMLListView::createItem(UMLListViewItem& Data, IDChangeLog& IDChanges,
					 UMLListViewItem* parent /*= 0*/) {
	UMLObject* pObject = 0;
	UMLView* v = 0;
	UMLListViewItem* item = 0;
	Uml::ListView_Type lvt = Data.getType();
	if(!parent) {
		if(lvt == Uml::lvt_Actor || lvt == Uml::lvt_UseCase || lvt == Uml::lvt_UseCase_Folder || lvt == Uml::lvt_UseCase_Diagram) {
			parent = ucv;
		} else if ( lvt == Uml::lvt_Component_Diagram ||
			    lvt == Uml::lvt_Component ||
			    lvt == Uml::lvt_Artifact) {
			parent = componentView;
		} else if ( lvt == Uml::lvt_Deployment_Diagram ||
			    lvt == Uml::lvt_Node) {
			parent = deploymentView;
		} else if( typeIsDiagram(lvt) ) {
			parent = lv;
		} else if( !typeIsClassifierList(lvt) ) {
			parent = lv;
		} else {
			return 0;
		}
	}
	int newID;

	switch(lvt) {
	case Uml::lvt_Actor:
	case Uml::lvt_UseCase:
	case Uml::lvt_Class:
	case Uml::lvt_Package:
	case Uml::lvt_Component:
	case Uml::lvt_Node:
	case Uml::lvt_Artifact:
	case Uml::lvt_Interface:
		newID = IDChanges.findNewID(Data.getID());
		//if there is no ListViewItem associated with the new ID,
		//it could exist an Item already asocciated if the user chose to reuse an uml object
		if(!(item = findItem(newID))) {
			pObject = m_doc->findUMLObject( IDChanges.findNewID(Data.getID()) );
			item = new UMLListViewItem(parent, Data.getLabel(), lvt, pObject);
		}
		break;
	case Uml::lvt_Logical_Folder:
	case Uml::lvt_UseCase_Folder:
	case Uml::lvt_Component_Folder:
	case Uml::lvt_Deployment_Folder:
		item = new UMLListViewItem(parent, Data.getLabel(), lvt);
		break;
	case Uml::lvt_Attribute:
	case Uml::lvt_Template:
	{
		UMLClass * pClass =  (UMLClass *)parent -> getUMLObject();
		newID = IDChanges.findNewID( Data.getID() );
		pObject = pClass -> findChildObject( newID );
		if (pObject) {
			item = new UMLListViewItem( parent, Data.getLabel(), lvt, pObject );
		} else {
			item = 0;
		}
		break;
	}
	case Uml::lvt_Operation:
	{
		UMLClassifier * pConcept =  (UMLClassifier *)parent -> getUMLObject();
		pObject = pConcept  -> findChildObject( IDChanges.findNewID( Data.getID() ) );
		if (pObject) {
			item = new UMLListViewItem( parent, Data.getLabel(), lvt, pObject );
		} else {
			item = 0;
		}
		break;
	}
	case Uml::lvt_UseCase_Diagram:
	case Uml::lvt_Sequence_Diagram:
	case Uml::lvt_Collaboration_Diagram:
	case Uml::lvt_Class_Diagram:
	case Uml::lvt_State_Diagram:
	case Uml::lvt_Activity_Diagram:
	case Uml::lvt_Component_Diagram:
	case Uml::lvt_Deployment_Diagram:
		v = m_doc->findView(IDChanges.findNewID(Data.getID()));
		if(!v) {
			return 0;
		}
		item = new UMLListViewItem(parent, v->getName(), convert_DT_LVT( v->getType() ),  IDChanges.findNewID(Data.getID()));
		break;
	default:
		kdWarning() << "createItem() called on unknown type" << endl;
		break;
	}
	return item;
}

int UMLListView::getSelectedCount() {
	QListViewItemIterator it(this);
	int count = 0;
	// iterate through all items of the list view
	for ( ; it.current(); ++it ) {
		if ( it.current()->isSelected() ) {
			count++;
		}
	}

	return count;
}

void UMLListView::focusOutEvent ( QFocusEvent * fe) {
	clearSelection();
	triggerUpdate();
	//repaint();

	QListView::focusOutEvent(fe);
}

Uml::ListView_Type UMLListView::convert_DT_LVT(Uml::Diagram_Type dt) {
	Uml::ListView_Type type =  Uml::lvt_Unknown;
	switch(dt) {
	case Uml::dt_UseCase:
		type = Uml::lvt_UseCase_Diagram;
		break;

	case Uml::dt_Class:
		type = Uml::lvt_Class_Diagram;
		break;

	case Uml::dt_Sequence:
		type = Uml::lvt_Sequence_Diagram;
		break;

	case Uml::dt_Collaboration:
		type = Uml::lvt_Collaboration_Diagram;
		break;

	case Uml::dt_State:
		type = Uml::lvt_State_Diagram;
		break;

	case Uml::dt_Activity:
		type = Uml::lvt_Activity_Diagram;
		break;

	case Uml::dt_Component:
		type = Uml::lvt_Component_Diagram;
		break;

	case Uml::dt_Deployment:
		type = Uml::lvt_Deployment_Diagram;
		break;

	default:
		kdWarning() << "convert_DT_LVT() called on unknown diagram type" << endl;
		break;
	}
	return type;
}

Uml::ListView_Type UMLListView::convert_OT_LVT(Uml::UMLObject_Type ot) {
	Uml::ListView_Type type =  Uml::lvt_Unknown;
	switch(ot) {
	case Uml::ot_UseCase:
		type = Uml::lvt_UseCase;
		break;

	case Uml::ot_Actor:
		type = Uml::lvt_Actor;
		break;

	case Uml::ot_Class:
		type = Uml::lvt_Class;
		break;

	case Uml::ot_Package:
		type = Uml::lvt_Package;
		break;

	case Uml::ot_Component:
		type = Uml::lvt_Component;
		break;

	case Uml::ot_Node:
		type = Uml::lvt_Node;
		break;

	case Uml::ot_Artifact:
		type = Uml::lvt_Artifact;
		break;

	case Uml::ot_Interface:
		type = Uml::lvt_Interface;
		break;

	case Uml::ot_Attribute:
		type = Uml::lvt_Attribute;
		break;

	case Uml::ot_Operation:
		type = Uml::lvt_Operation;
		break;

	case Uml::ot_Template:
		type = Uml::lvt_Template;
		break;
	default:
		break;
	}
	return type;
}

QPixmap & UMLListView::getPixmap( Icon_Type type ) {
	switch( type ) {
	case it_Home:
		return m_Pixmaps.Home;
		break;

	case it_Folder_Green:
		return m_Pixmaps.Folder_Green;
		break;

	case it_Folder_Green_Open:
		return m_Pixmaps.Folder_Green_Open;
		break;

	case it_Folder_Grey:
		return m_Pixmaps.Folder_Grey;
		break;

	case it_Folder_Grey_Open:
		return m_Pixmaps.Folder_Grey_Open;
		break;

	case it_Folder_Red:
		return m_Pixmaps.Folder_Red;
		break;

	case it_Folder_Red_Open:
		return m_Pixmaps.Folder_Red_Open;
		break;

	case it_Folder_Violet:
		return m_Pixmaps.Folder_Violet;
		break;

	case it_Folder_Violet_Open:
		return m_Pixmaps.Folder_Violet_Open;
		break;

	case it_Diagram:
		return m_Pixmaps.Diagram;
		break;

	case it_Class:
		return m_Pixmaps.Class;
		break;

	case it_Template:
		return m_Pixmaps.Template;
		break;

	case it_Package:
		return m_Pixmaps.Package;
		break;

	case it_Component:
		return m_Pixmaps.Component;
		break;

	case it_Node:
		return m_Pixmaps.Node;
		break;

	case it_Artifact:
		return m_Pixmaps.Artifact;
		break;

	case it_Interface:
		return m_Pixmaps.Interface;
		break;

	case it_Actor:
		return m_Pixmaps.Actor;
		break;

	case it_UseCase:
		return m_Pixmaps.UseCase;
		break;

	case it_Public_Method:
		return m_Pixmaps.Public_Method;
		break;

	case it_Private_Method:
		return m_Pixmaps.Private_Method;
		break;

	case it_Protected_Method:
		return m_Pixmaps.Protected_Method;
		break;

	case it_Public_Attribute:
		return m_Pixmaps.Public_Attribute;
		break;

	case it_Private_Attribute:
		return m_Pixmaps.Private_Attribute;
		break;

	case it_Protected_Attribute:
		return m_Pixmaps.Protected_Attribute;
		break;

	default:
		kdWarning() << "getPixmap() called on unknown icon" << endl;
		break;
	}//end switch
	return m_Pixmaps.Home;//you'll know you have a problem if this shows up in the list!!
}

void UMLListView::loadPixmaps() {
	KStandardDirs * dirs = KGlobal::dirs();
	QString dataDir = dirs -> findResourceDir("data", "umbrello/pics/object.xpm");
	dataDir += "/umbrello/pics/";

	m_Pixmaps.Home = SmallIcon("folder_home");
	m_Pixmaps.Folder_Green = SmallIcon("folder_green");
	m_Pixmaps.Folder_Green_Open = SmallIcon("folder_green_open");
	m_Pixmaps.Folder_Grey = SmallIcon("folder_grey");
	m_Pixmaps.Folder_Grey_Open = SmallIcon("folder_grey_open");
	m_Pixmaps.Folder_Red = SmallIcon("folder_red");
	m_Pixmaps.Folder_Red_Open = SmallIcon("folder_red_open");
	m_Pixmaps.Folder_Violet = SmallIcon("folder_violet");
	m_Pixmaps.Folder_Violet_Open = SmallIcon("folder_violet_open");

	m_Pixmaps.Diagram.load( dataDir + "CVnamespace.png" ); //change to have different one for each type of diagram
	m_Pixmaps.Class.load( dataDir + "umlclass.xpm" );
	m_Pixmaps.Template.load( dataDir + "umlclass_template.xpm" );
	m_Pixmaps.Package.load( dataDir + "package.xpm" );
	m_Pixmaps.Component.load( dataDir + "component.xpm" );
	m_Pixmaps.Node.load( dataDir + "node.xpm" );
	m_Pixmaps.Artifact.load( dataDir + "artifact.xpm" );
	m_Pixmaps.Interface.load( dataDir + "interface.xpm" );
	m_Pixmaps.Actor.load( dataDir + "actor.xpm" );
	m_Pixmaps.UseCase.load( dataDir + "case.xpm" );
	m_Pixmaps.Public_Method.load( dataDir + "CVpublic_meth.png" );
	m_Pixmaps.Private_Method.load( dataDir + "CVprivate_meth.png" );
	m_Pixmaps.Protected_Method.load( dataDir + "CVprotected_meth.png" );
	m_Pixmaps.Public_Attribute.load( dataDir + "CVpublic_var.png" );
	m_Pixmaps.Private_Attribute.load( dataDir + "CVprivate_var.png" );
	m_Pixmaps.Protected_Attribute.load( dataDir + "CVprotected_var.png" );
}

void UMLListView::slotExpanded( QListViewItem * item ) {
	UMLListViewItem * myItem= (UMLListViewItem *)item;
	switch( myItem -> getType() ) {
	case Uml::lvt_Logical_View:
	case Uml::lvt_UseCase_View:
	case Uml::lvt_Component_View:
	case Uml::lvt_Deployment_View:
	case Uml::lvt_Logical_Folder:
	case Uml::lvt_UseCase_Folder:
	case Uml::lvt_Component_Folder:
	case Uml::lvt_Deployment_Folder:
		myItem->updateFolder();
		break;
	default:
		break;
	}//end switch
}

void UMLListView::slotCollapsed( QListViewItem * item ) {
	UMLListViewItem * myItem = (UMLListViewItem *)item;
	switch( myItem -> getType() ) {
	case Uml::lvt_Logical_View:
	case Uml::lvt_UseCase_View:
	case Uml::lvt_Component_View:
	case Uml::lvt_Deployment_View:
	case Uml::lvt_Logical_Folder:
	case Uml::lvt_UseCase_Folder:
	case Uml::lvt_Component_Folder:
	case Uml::lvt_Deployment_Folder:
		myItem -> updateFolder();
		break;
	default:
		break;
	}//end switch
}

void UMLListView::slotCutSuccessful() {
	if( m_bStartedCut ) {
		popupMenuSel( ListPopupMenu::mt_Delete );
		//deletion code here
		m_bStartedCut = false;
	}
}

void UMLListView::addNewItem( QListViewItem * parent, Uml::ListView_Type type ) {
	QString name = i18n("folder");
	UMLListViewItem * newItem = static_cast<UMLListViewItem *>( parent );//used for att/ops - rest overwrite
	parent -> setOpen( true );
	switch( type ) {
	case Uml::lvt_UseCase_Folder:
	case Uml::lvt_Logical_Folder:
	case Uml::lvt_Component_Folder:
	case Uml::lvt_Deployment_Folder:
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, -1 );
		break;

	case Uml::lvt_Actor:
		name = getUniqueUMLObjectName( Uml::ot_Actor );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem -> setPixmap( 0, getPixmap( it_Actor ) );
		break;

	case Uml::lvt_Class:
		name = getUniqueUMLObjectName( Uml::ot_Class);
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem -> setPixmap( 0, getPixmap( it_Class ) );
		break;

	case Uml::lvt_Package:
		name = getUniqueUMLObjectName( Uml::ot_Package );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem->setPixmap( 0, getPixmap( it_Package ) );
		break;

	case Uml::lvt_Component:
		name = getUniqueUMLObjectName( Uml::ot_Component );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem->setPixmap( 0, getPixmap( it_Component ) );
		break;

	case Uml::lvt_Node:
		name = getUniqueUMLObjectName( Uml::ot_Node );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem->setPixmap( 0, getPixmap( it_Node ) );
		break;

	case Uml::lvt_Artifact:
		name = getUniqueUMLObjectName( Uml::ot_Artifact );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem->setPixmap( 0, getPixmap( it_Artifact ) );
		break;

	case Uml::lvt_Interface:
		name = getUniqueUMLObjectName( Uml::ot_Interface );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem*>(parent), name, type, (UMLObject*)0 );
		newItem->setPixmap( 0, getPixmap( it_Interface ) );
		break;

	case Uml::lvt_Attribute:
	{
		UMLClass * childParent = static_cast<UMLClass*>( newItem->getUMLObject() );
		name = getUniqueChildUMLObjectName( childParent, Uml::ot_Attribute );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem -> setPixmap( 0, getPixmap( it_Public_Attribute ) );
		break;
	}
	case Uml::lvt_Operation:
	{
		UMLClassifier * childParent = static_cast<UMLClassifier *>( newItem->getUMLObject() );
		name = getUniqueChildUMLObjectName( childParent, Uml::ot_Operation );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem -> setPixmap( 0, getPixmap( it_Public_Method ) );
		break;
	}
	case Uml::lvt_UseCase:
		name = getUniqueUMLObjectName( Uml::ot_UseCase );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, (UMLObject *)0 );
		newItem -> setPixmap( 0, getPixmap( it_UseCase ) );
		break;

	case Uml::lvt_Class_Diagram:
		name = getUniqueDiagramName( Uml::dt_Class );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, -1 );
		break;

	case Uml::lvt_UseCase_Diagram:
		name = getUniqueDiagramName( Uml::dt_UseCase );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, -1 );
		break;

	case Uml::lvt_Sequence_Diagram:
		name = getUniqueDiagramName( Uml::dt_Sequence );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, -1 );
		break;

	case Uml::lvt_Collaboration_Diagram:
		name = getUniqueDiagramName( Uml::dt_Collaboration );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, -1 );
		break;

	case Uml::lvt_State_Diagram:
		name = getUniqueDiagramName( Uml::dt_State );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, -1 );
		break;
	case Uml::lvt_Activity_Diagram:
		name = getUniqueDiagramName( Uml::dt_Activity );
		newItem = new UMLListViewItem( static_cast<UMLListViewItem *>( parent ), name, type, -1 );
		break;

	case Uml::lvt_Component_Diagram:
		name = getUniqueDiagramName(Uml::dt_Component);
		newItem = new UMLListViewItem(static_cast<UMLListViewItem*>(parent), name, type, -1);
		break;

	case Uml::lvt_Deployment_Diagram:
		name = getUniqueDiagramName(Uml::dt_Deployment);
		newItem = new UMLListViewItem(static_cast<UMLListViewItem*>(parent), name, type, -1);
		break;
	default:
		break;
	}
	m_bIgnoreCancelRename = false;
	ensureItemVisible(newItem);
	newItem->setOpen( true );
	newItem->setCreating( true );
	newItem->startRename( 0 );
}

bool UMLListView::slotItemRenamed( QListViewItem * item , int /*col*/ ) {
	//if true the item was cancel before this message
	if( m_bIgnoreCancelRename ) {
		return true;
	}
	m_bIgnoreCancelRename = true;
	UMLListViewItem * renamedItem = static_cast< UMLListViewItem *>( item ) ;
	Uml::ListView_Type type = renamedItem -> getType();
	QString newText = renamedItem -> text( 0 );
	renamedItem -> setCreating( false );

	//if the length of any type then delete it.
	if( newText.length() == 0 ) {
		KMessageBox::error( kapp -> mainWidget() , i18n( "The name you entered was invalid!\nCreation process has been canceled." ),
		                    i18n( "Name Not Valid" ) );
		delete item;
		return false;
	}
	// No need to do anything for folders if the string length was > 0
	if( typeIsFolder(type) ) {
		return true;
	}

	if( !isUnique( renamedItem, newText ) ) {
		//if operation ask if ok not to be unique i.e overloading
		if( type == Uml::lvt_Operation ) {
			if( KMessageBox::warningYesNo( kapp -> mainWidget() , i18n( "The name you entered was not unique!\nIs this what you wanted?" ),
			                               i18n( "Name Not Unique" ) ) == KMessageBox::No ) {
				delete item;
				return false;
			}
		} else {
			KMessageBox::error( kapp -> mainWidget() , i18n( "The name you entered was not unique!\nCreation process has been canceled." ),
			                    i18n( "Name Not Unique" ) );
			delete item;
			return false;
		}
	}

	switch( type ) {
	case Uml::lvt_Actor:
		createUMLObject( renamedItem, Uml::ot_Actor );
		break;

	case Uml::lvt_Class:
		createUMLObject( renamedItem, Uml::ot_Class);
		break;

	case Uml::lvt_Package:
		createUMLObject( renamedItem, Uml::ot_Package );
		break;

	case Uml::lvt_Component:
		createUMLObject(renamedItem, Uml::ot_Component);
		break;

	case Uml::lvt_Node:
		createUMLObject(renamedItem, Uml::ot_Node);
		break;

	case Uml::lvt_Artifact:
		createUMLObject(renamedItem, Uml::ot_Artifact);
		break;

	case Uml::lvt_Interface:
		createUMLObject( renamedItem, Uml::ot_Interface );
		break;

	case Uml::lvt_UseCase:
		createUMLObject( renamedItem, Uml::ot_UseCase );
		break;

	case Uml::lvt_Attribute:
		createChildUMLObject( renamedItem, Uml::ot_Attribute );
		break;

	case Uml::lvt_Operation:
		createChildUMLObject( renamedItem, Uml::ot_Operation );
		break;

	case Uml::lvt_Class_Diagram:
		createDiagram( renamedItem, Uml::dt_Class );
		break;

	case Uml::lvt_UseCase_Diagram:
		createDiagram( renamedItem, Uml::dt_UseCase );
		break;

	case Uml::lvt_Sequence_Diagram:
		createDiagram( renamedItem, Uml::dt_Sequence );
		break;

	case Uml::lvt_Collaboration_Diagram:
		createDiagram( renamedItem, Uml::dt_Collaboration );
		break;

	case Uml::lvt_State_Diagram:
		createDiagram( renamedItem, Uml::dt_State );
		break;

	case Uml::lvt_Activity_Diagram:
		createDiagram( renamedItem, Uml::dt_Activity );
		break;

	case Uml::lvt_Component_Diagram:
		createDiagram( renamedItem, Uml::dt_Component );
		break;

	case Uml::lvt_Deployment_Diagram:
		createDiagram( renamedItem, Uml::dt_Deployment );
		break;

	default:
		break;

	}
	return true;
}

void UMLListView::createUMLObject( UMLListViewItem * item, Uml::UMLObject_Type type ) {
	QString name = item -> text( 0 );
	UMLObject * object = m_doc -> findUMLObject( type, name );
	if( object ) {
		delete item;
		return;
	}
	switch( type ) {
	case Uml::ot_UseCase:
		object = new UMLUseCase( m_doc, name, m_doc -> getUniqueID() );
		break;

	case Uml::ot_Actor:
		object = new UMLActor( m_doc, name, m_doc -> getUniqueID() );
		break;

	case Uml::ot_Class:
		object = new UMLClass( m_doc, name, m_doc -> getUniqueID() );
		break;

	case Uml::ot_Package:
		object = new UMLPackage( m_doc, name, m_doc->getUniqueID() );
		break;

	case Uml::ot_Component:
		object = new UMLComponent( m_doc, name, m_doc->getUniqueID() );
		break;

	case Uml::ot_Node:
		object = new UMLNode( m_doc, name, m_doc->getUniqueID() );
		break;

	case Uml::ot_Artifact:
		object = new UMLArtifact( m_doc, name, m_doc->getUniqueID() );
		break;

	case Uml::ot_Interface:
		object = new UMLInterface( m_doc, name, m_doc->getUniqueID() );
		break;
	default:
		kdWarning() << "createing UML Object of unknown type" << endl;
		break;
	}

	connectNewObjectsSlots(object);
	m_doc -> addUMLObject( object );
	item -> setUMLObject( object );
	item -> setLabel( name );
}

void UMLListView::createChildUMLObject( UMLListViewItem * item, Uml::UMLObject_Type type ) {
	m_bCreatingChildObject = true;
	QString name = item -> text( 0 );
	UMLObject * object = static_cast<UMLListViewItem *>( item -> parent() ) -> getUMLObject();
	if( !object ) {
		delete item;
		return;
	}

	if( type == Uml::ot_Attribute )
	{
		UMLClass * parent = static_cast<UMLClass *>( object );
		object = parent -> addAttribute( name, m_doc -> getUniqueID() );
	}
	else
	{
		UMLClassifier * parent = static_cast<UMLClassifier *>( object );
		object = parent -> addOperation( name, m_doc -> getUniqueID() );
	}

	m_doc -> addUMLObject( object );

	item -> setUMLObject( object );
	item -> setLabel( name );
	m_bCreatingChildObject = false;
}

void UMLListView::createDiagram( UMLListViewItem * item, Uml::Diagram_Type type ) {
	QString name = item -> text( 0 );
	UMLView * view = m_doc -> findView( type, name );
	if( view ) {
		delete item;
		return;
	}
	view = new UMLView(UMLApp::app()->getMainDockWidget(), m_doc);
	view->setName( name );
	view->setType( type );
	view->setID( m_doc -> getUniqueID() );
	m_doc -> addView( view );
	view  -> setOptionState( ((UMLApp *) m_doc -> parent()) -> getOptionState() );
	item -> setID( view -> getID() );
	item -> setLabel( name );
	view->activate();
	m_doc -> changeCurrentView( view -> getID() );
}

QString UMLListView::getUniqueUMLObjectName( Uml::UMLObject_Type type ) {
	QString temp = "";
	QString name = "";
	QString newClass = i18n("new_class");
	QString newPackage = i18n("new_package");
	QString newComponent = i18n("new_component");
	QString newNode = i18n("new_node");
	QString newArtifact = i18n("new_artifact");
	QString newInterface = i18n("new_interface");
	QString newActor = i18n("new_actor");
	QString newUseCase = i18n("new_usecase");

	UMLObject * object = 0;
	int count = 0;

	if (type == Uml::ot_Actor) {
		name = newActor;
	} else if (type == Uml::ot_UseCase) {
		name = newUseCase;
	} else if (type == Uml::ot_Class) {
		name = newClass;
	} else if (type == Uml::ot_Package) {
		name = newPackage;
	} else if (type == Uml::ot_Component) {
		name = newComponent;
	} else if (type == Uml::ot_Node) {
		name = newNode;
	} else if (type == Uml::ot_Artifact) {
		name = newArtifact;
	} else if (type == Uml::ot_Interface) {
		name = newInterface;
	} else {
		kdWarning() << "getting unique uml object name for unknown type" << endl;
	}

	do {
		temp = name;
		if( count > 0 )
			temp.append("_").append( QString::number( count ) );
		object = m_doc -> findUMLObject( type, temp );
		count++;
	} while( object );

	return temp;
}

QString UMLListView::getUniqueChildUMLObjectName( UMLClassifier * parent, Uml::UMLObject_Type type ) {
	QString name = "";
	QString temp = "";
	QString newAtt = i18n("new_attribute");
	QString newOp = i18n("new_operation");

	bool cont = true;
	int count = 0;

	if( type == Uml::ot_Attribute )
		name = newAtt;
	else
		name = newOp;

	do {
		temp = name;
		if( count > 0 )
			temp.append("_").append( QString::number( count ) );
		cont = parent -> findChildObject( type, temp ).count() != 0;
		count++;
	} while( cont );
	return temp;
}

QString UMLListView::getUniqueDiagramName(Uml::Diagram_Type type) {
	return m_doc->uniqViewName(type);
}

bool UMLListView::isUnique( UMLListViewItem * item, QString name ) {
	UMLListViewItem * parentItem = static_cast<UMLListViewItem *>( item -> parent() );
	Uml::ListView_Type type = item -> getType();
	switch( type ) {
	case Uml::lvt_Class_Diagram:
		return !m_doc -> findView( Uml::dt_Class, name );
		break;

	case Uml::lvt_Sequence_Diagram:
		return !m_doc -> findView( Uml::dt_Sequence, name );
		break;

	case Uml::lvt_UseCase_Diagram:
		return !m_doc -> findView( Uml::dt_UseCase, name );
		break;

	case Uml::lvt_Collaboration_Diagram:
		return !m_doc -> findView( Uml::dt_Collaboration, name );
		break;

	case Uml::lvt_State_Diagram:
		return !m_doc -> findView( Uml::dt_State, name );
		break;

	case Uml::lvt_Activity_Diagram:
		return !m_doc -> findView( Uml::dt_Activity, name );
		break;

	case Uml::lvt_Component_Diagram:
		return !m_doc->findView(Uml::dt_Component, name);
		break;

	case Uml::lvt_Deployment_Diagram:
		return !m_doc->findView(Uml::dt_Deployment, name);
		break;

	case Uml::lvt_Actor:
		return !m_doc -> findUMLObject( Uml::ot_Actor, name );
		break;

	case Uml::lvt_UseCase:
		return !m_doc -> findUMLObject( Uml::ot_UseCase, name );
		break;

	case Uml::lvt_Class:
		return !m_doc -> findUMLObject( Uml::ot_Class, name );
		break;

	case Uml::lvt_Package:
		return !m_doc->findUMLObject(Uml::ot_Package, name);
		break;

	case Uml::lvt_Component:
		return !m_doc->findUMLObject(Uml::ot_Component, name);
		break;

	case Uml::lvt_Node:
		return !m_doc->findUMLObject(Uml::ot_Node, name);
		break;

	case Uml::lvt_Artifact:
		return !m_doc->findUMLObject(Uml::ot_Artifact, name);
		break;

	case Uml::lvt_Interface:
		return !m_doc->findUMLObject(Uml::ot_Interface, name);
		break;

	case Uml::lvt_Attribute:
	{
		UMLClass * parent = static_cast<UMLClass *>( parentItem  -> getUMLObject() );
		return ( parent -> findChildObject( Uml::ot_Attribute, name ).count() == 0 );
		break;
	}
	case Uml::lvt_Operation:
	{
		UMLClassifier * parent = static_cast<UMLClassifier *>( parentItem  -> getUMLObject() );
		return ( parent -> findChildObject( Uml::ot_Operation, name ).count() == 0 );
		break;
	}
	default:
		break;
	}
	return false;
}

void UMLListView::cancelRename( QListViewItem * item ) {
	if( !m_bIgnoreCancelRename ) {
		delete item;
		m_bIgnoreCancelRename = true;
	}
}

bool UMLListView::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement listElement = qDoc.createElement( "listview" );
	bool status = rv -> saveToXMI( qDoc, listElement);
	qElement.appendChild( listElement );
	return status;
}

bool UMLListView::loadFromXMI( QDomElement & element ) {
	deleteChildrenOf( ucv );
	deleteChildrenOf( lv );
	deleteChildrenOf( componentView );
	deleteChildrenOf( deploymentView );
//Uncomment for using Luis diagram display code
//	deleteChildrenOf( diagramFolder );

	QDomNode node = element.firstChild();
	QDomElement domElement = node.toElement();
	while( !domElement.isNull() ) {
		if( domElement.tagName() == "listitem" ) {
			QString type = domElement.attribute( "type", "-1" );
			if( type == "-1" )
				return false;
			Uml::ListView_Type lvType = (Uml::ListView_Type)type.toInt();
			if( lvType == Uml::lvt_View ) {
				if( !loadChildrenFromXMI( rv, domElement ) )
					return false;
			} else
				return false;
		}
		node = node.nextSibling();
		domElement = node.toElement();

	}//end while
	return true;
}

bool UMLListView::loadChildrenFromXMI( UMLListViewItem * parent, QDomElement & element ) {
	QDomNode node = element.firstChild();
	QDomElement domElement = node.toElement();
	while( !domElement.isNull() ) {
		node = domElement.nextSibling();
		if( domElement.tagName() != "listitem" ) {
			domElement = node.toElement();
			continue;
		}
		QString id = domElement.attribute( "id", "-1" );
		QString type = domElement.attribute( "type", "-1" );
		QString label = domElement.attribute( "label", "" );
		QString open = domElement.attribute( "open", "1" );
		if( type == "-1" )
			return false;
		Uml::ListView_Type lvType = (Uml::ListView_Type)type.toInt();
		bool bOpen = (bool)open.toInt();
		int nID = id.toInt();
		UMLObject * pObject = 0;
		UMLListViewItem * item = 0;
		if (nID != -1) {
			pObject = m_doc->findUMLObject(nID);
			if (pObject) {
				connectNewObjectsSlots(pObject);
			}
		}

		switch( lvType ) {
			case Uml::lvt_Class:
				item = new UMLListViewItem(parent, label, lvType, pObject);
				break;
			case Uml::lvt_Interface:
				item = new UMLListViewItem(parent, label, lvType, pObject);
				break;
			case Uml::lvt_Actor:
			case Uml::lvt_UseCase:
			case Uml::lvt_Package:
			case Uml::lvt_Component:
			case Uml::lvt_Node:
			case Uml::lvt_Artifact:
				item = new UMLListViewItem(parent, label, lvType, pObject);
				break;
			case Uml::lvt_Attribute:
			case Uml::lvt_Template:
			case Uml::lvt_Operation:
			{
				UMLObject* umlObject = parent->getUMLObject();
				if (!umlObject)
					return false;
				if (nID == -1) {
					kdWarning() << "UMLListView::loadChildrenFromXMI: lvtype " << lvType
						    << " has id -1" << endl;
				} else {
					UMLClass *classObj = dynamic_cast<UMLClass *>(umlObject);
					if (classObj) {
						umlObject = classObj->findChildObject(nID);
						if (umlObject) {
							connectNewObjectsSlots(umlObject);
							item = new UMLListViewItem( parent, label, lvType, umlObject);
						} else {
							kdDebug() << "UMLListView::loadChildrenFromXMI: "
								  << " lvtype " << lvType << " child object " << nID
								  << " not found" << endl;
						}
					} else {
						kdDebug() << "UMLListView::loadChildrenFromXMI: "
							  << "cast to class object failed" << endl;
					}
				}
				break;
			}
			case Uml::lvt_Logical_View:
				item = lv;
				break;
			case Uml::lvt_UseCase_View:
				item = ucv;
				break;
			case Uml::lvt_Component_View:
				item = componentView;
				break;
			case Uml::lvt_Deployment_View:
				item = deploymentView;
				break;

			case Uml::lvt_Diagrams:
//Uncomment for using Luis diagram display code
//				item = diagramFolder;
				break;

			default:
				if (nID != -1) {
					item = new UMLListViewItem( parent, label, lvType, nID );
				}
				break;
		}//end switch

		if (item)  {
			item->setOpen( (bool)bOpen );
			if ( !loadChildrenFromXMI(item, domElement) ) {
				return false;
			}
		} else {
			kdWarning() << "unused list view item " << nID
				    << " of lvtype " << lvType << endl;
		}
		domElement = node.toElement();
	}//end while
	return true;
}

/** Open all items in the list view*/
void UMLListView::expandAll(QListViewItem *item) {
	if(!item) item = firstChild();
	for( item = item->firstChild(); item; item = item->nextSibling())
		item->setOpen(true);
}
/** Close all items in the list view*/
void UMLListView::collapseAll(QListViewItem *item) {
	if(!item) item = firstChild();
	for( item = item->firstChild(); item; item = item->nextSibling())
		item->setOpen(false);
}

void UMLListView::setStartedCut(bool startedCut) {
	m_bStartedCut = startedCut;
}

bool UMLListView::typeIsCanvasWidget(ListView_Type type) {
	if (type == lvt_Actor ||
	    type == lvt_UseCase ||
	    type == lvt_Class ||
	    type == lvt_Package ||
	    type == lvt_Component ||
	    type == lvt_Node ||
	    type == lvt_Artifact ||
	    type == lvt_Interface) {
		return true;
	} else {
		return false;
	}
}

bool UMLListView::typeIsFolder(ListView_Type type) {
	if (type == Uml::lvt_Logical_Folder ||
	    type == Uml::lvt_UseCase_Folder ||
	    type == Uml::lvt_Component_Folder ||
	    type == Uml::lvt_Deployment_Folder) {
		return true;
	} else {
		return false;
	}
}

bool UMLListView::typeIsClassifierList(ListView_Type type) {
	if (type == Uml::lvt_Attribute ||
	    type == Uml::lvt_Operation ||
	    type == Uml::lvt_Template) {
		return true;
	} else {
		return false;
	}
}

bool UMLListView::typeIsDiagram(ListView_Type type) {
	if (type == Uml::lvt_Class_Diagram ||
	    type == Uml::lvt_Collaboration_Diagram ||
	    type == Uml::lvt_State_Diagram ||
	    type == Uml::lvt_Activity_Diagram ||
	    type == Uml::lvt_Sequence_Diagram ||
	    type == Uml::lvt_UseCase_Diagram ||
	    type == Uml::lvt_Component_Diagram ||
	    type == Uml::lvt_Deployment_Diagram) {
		return true;
	} else {
		return false;
	}
}

void UMLListView::deleteChildrenOf(QListViewItem* parent) {
	if ( !parent ) {
		return;
	}

	while ( parent->firstChild() ) {
		delete parent->firstChild();
	}
}

#include "umllistview.moc"

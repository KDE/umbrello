/***************************************************************************
                          refactoringassistant.cpp
                             -------------------
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "refactoringassistant.h"

#include "../umlnamespace.h"
#include "../umldoc.h"
#include "../classifier.h"
#include "../class.h"
#include "../interface.h"
#include "../attribute.h"
#include "../operation.h"

#include "../dialogs/operationpropertiespage.h"

#include <qpoint.h>
#include <qpopupmenu.h>

#include <typeinfo>
#include <kstddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <klineeditdlg.h>
#include <kdebug.h>

using std::typeinfo;
 
 
 namespace Umbrello{
 
 
RefactoringAssistant::RefactoringAssistant( UMLDoc *doc, UMLClassifier *obj, QWidget *parent, const char *name ):
		KListView( parent, name ), m_doc( doc )
{
	loadPixmaps();

	setRootIsDecorated(true);	
	setAcceptDrops( true );
	setDropVisualizer( false );
	setItemsMovable( true );
	setSelectionModeExt( Single );
	setShowToolTips( true );
	setTooltipColumn( 0 );
	setDragEnabled( true );
	setDropHighlighter( true );
	setFullWidth( true );
	setSorting( -1 );
	
	addColumn("Name ");

	setObject( obj );
	
	m_menu = new QPopupMenu(this);
	
	connect(this,SIGNAL(doubleClicked(QListViewItem*)),this,SLOT(itemExecuted(QListViewItem*)));
	connect(this,SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
		this,SLOT(showContextMenu(KListView*,QListViewItem*,const QPoint&)));
		
	resize(300,400);

}

RefactoringAssistant::~RefactoringAssistant()
{
}

void RefactoringAssistant::itemExecuted( QListViewItem *item )
{
	if( m_umlObjectMap.find(item) == m_umlObjectMap.end() )
	{
		kdDebug()<<"item with text "<<item->text(0)<<"not found in uml map!"<<endl;
		return;
	}
	QWidget *page(0);
	UMLObject *obj = m_umlObjectMap[item];
	if(typeid(*obj) == typeid(UMLOperation))
	{
		page = new OperationPropertiesPage( static_cast<UMLOperation*>(obj), 0, "operation properties");
	}
	if(typeid(*obj) == typeid(UMLAttribute))
	{
		//page = new AttributePropertiesPage( static_cast<UMLAttribute*>(obj), 0, "attribute properties");
	}
	if( page )
	{
		page->show();
	}
	
}

void RefactoringAssistant::showContextMenu(KListView *v,QListViewItem *item, const QPoint &p)
{
	m_menu->clear();
	if( m_umlObjectMap.find(item) == m_umlObjectMap.end() )
	{
		kdDebug()<<"no context menu for item "<<item->text(0)<<endl;;
		return;
	}
	UMLObject *obj = m_umlObjectMap[item];
	if(typeid(*obj) == typeid(UMLClass))
	{
		m_menu->insertItem(i18n("Add Superclass"),this,SLOT(addSuperClassifier()));
		m_menu->insertItem(i18n("Add derived class"),this,SLOT(addDerivedClassifier()));
		m_menu->insertItem(i18n("Add interface implementation"),this,SLOT(addInterfaceImplementation()));
	}
	else if(typeid(*obj) == typeid(UMLInterface))
	{
		m_menu->insertItem(i18n("Add superinterface"),this,SLOT(addSuperClassifier()));
		m_menu->insertItem(i18n("Add derived interface"),this,SLOT(addDerivedClassifier()));
	}
	else
	{
		kdDebug()<<"No context menu for objects of type "<<typeid(*obj).name()<<endl;
		return;
	}
	m_menu->exec(p);
	
}

void RefactoringAssistant::addSuperClassifier()
{
	QString text, name;
	bool inputOk;
	
	QListViewItem *item;
	for( item = firstChild(); item != 0; item = item->itemBelow() )
	{
		if( item->isSelected() )
			break;
	}
	if( m_umlObjectMap.find(item) == m_umlObjectMap.end() )
	{
		kdWarning()<<"addSuperClassifier() : no uml object found for item "<<item->text(0)<<endl;;
		return;
	}
	UMLObject *obj = m_umlObjectMap[item];
	UMLObject_Type type;
	if(typeid(*obj) == typeid(UMLClass))
	{
		text = i18n("Enter a name for exisiting or new super class");
		type = ot_Class;
	}
	else if(typeid(*obj) == typeid(UMLInterface))
	{
		text = i18n("Enter a name for exisiting or new super interface");
		type = ot_Interface;
	}
	name = KLineEditDlg::getText(text, QString::null ,&inputOk,this);
	if(!inputOk)
	{
		return;
	}
	
	UMLObject *super = m_doc->findUMLObject( type, name );
	if(!super)
	{
		super = m_doc->createUMLObject( type, name );
	}
	if(!super)
	{
		return;
	}
	m_doc->createUMLAssociation(super, obj, at_Generalization);
}

void RefactoringAssistant::addDerivedClassifier()
{

}

void RefactoringAssistant::addInterfaceImplementation()
{

}


void RefactoringAssistant::addClassifier( UMLClassifier *classifier, QListViewItem *parent, bool addSuper, bool addSub, bool recurse)
{
	QListViewItem *classifierItem, *attsFolder, *opsFolder, *superFolder, *derivedFolder, *item;
	if( parent )
	{
		classifierItem = parent;
	}
	else
	{
		classifierItem= new KListViewItem( this, classifier->getName() );
		m_umlObjectMap[classifierItem] = classifier;
	}
	
	//only classes have attributes, interfaces only have operations
	if(typeid(*classifier) == typeid (UMLClass))
	{	//column 0 = visible string, column 1 = type (hidden)
		attsFolder = new KListViewItem( classifierItem, i18n("Attributes"), "attributes" );
		attsFolder->setPixmap(0,SmallIcon("folder_green_open"));
		attsFolder->setExpandable( true );
		QPtrList<UMLAttribute> *atts = static_cast<UMLClass*>(classifier)->getAttList( );
		for( UMLAttribute *att = atts->first(); att ; att = atts->next() )
		{
			item = new KListViewItem( attsFolder, att->getName() );
			m_umlObjectMap[item] = att;
			switch(att->getScope())
			{
				case Uml::Public:
					item->setPixmap(0,m_pixmaps.Public);
					break;
				case Uml::Protected:
					item->setPixmap(0,m_pixmaps.Protected);
					break;
				case Uml::Private:
					item->setPixmap(0,m_pixmaps.Private);
					break;
			}
		}
		
	}
	
	// add operations
	opsFolder = new KListViewItem( classifierItem, i18n("Operations"), "operations" );
	opsFolder->setPixmap(0,SmallIcon("folder_blue_open"));
	attsFolder->setExpandable( true );
	QPtrList<UMLOperation> *ops = classifier->getOpList( );
	for( UMLOperation *op = ops->first(); op ; op = ops->next() )
	{
		item = new KListViewItem( opsFolder, op->getName() );
		m_umlObjectMap[item] = op;
		switch(op->getScope())
		{
			case Uml::Public:
				item->setPixmap(0,m_pixmaps.Public);
				break;
			case Uml::Protected:
				item->setPixmap(0,m_pixmaps.Protected);
				break;
			case Uml::Private:
				item->setPixmap(0,m_pixmaps.Private);
				break;
		}
	}
	
	//if add parents
	if(addSuper)
	{
	superFolder = new KListViewItem( classifierItem, i18n("Super") );
	QPtrList<UMLClassifier> super = classifier->findSuperClassConcepts( m_doc );
	for( UMLClassifier *cl = super.first(); cl ; cl = super.next() )
	{
		item = new KListViewItem( superFolder, cl->getName() );
		item->setPixmap(0,m_pixmaps.Generalization);
		item->setExpandable( true );
		m_umlObjectMap[item] = cl;
		if( recurse )
		{
			addClassifier( cl, item, true, false, true);
		}
		
	}
	}
	if(addSub)
	{
	//add derived classifiers
	derivedFolder = new KListViewItem( classifierItem, i18n("Derived") );
	QPtrList<UMLClassifier> derived = classifier->findSubClassConcepts ( m_doc );
	for( UMLClassifier *d = derived.first(); d ; d = derived.next() )
	{
		item = new KListViewItem( derivedFolder, d->getName() );
		item->setPixmap(0,m_pixmaps.Subclass);
		item->setExpandable( true );
		m_umlObjectMap[item] = d;
		if( recurse )
		{
			addClassifier( d, item, false, true, true);
		}
		
	}
	}
}
void RefactoringAssistant::setObject( UMLClassifier *obj )
{
//check if we need to save/apply anything..
	clear();
	m_umlObject = obj;
	if (! m_umlObject )
	{
		//kdDebug()<<" null object!"<<endl;
		return;
	}
	//clear the map!
	addClassifier( obj, 0, true, true, true );
	
}

bool RefactoringAssistant::acceptDrag(QDropEvent *event) const
{
	//first check if we can accept drops at all, and if the operation
	// is a move within the list itself
	if( !acceptDrops() || !itemsMovable() || (event->source()!=viewport()))
	{
		return false;
	}
	
	RefactoringAssistant *me = const_cast<RefactoringAssistant*>(this);
	if(!me)
	{
		return false;
	}
	
	//ok, check if the move is valid
	QListViewItem *moving;
	QListViewItem *afterme;
	QListViewItem *parent;
	me->findDrop(event->pos(), parent, afterme);
	for( moving = firstChild(); moving != 0; moving = moving->itemBelow() )
	{
		if( moving->isSelected() )
			break;
	}
	if(!moving)
		return false;
	
	//check the uml objects represented by the items
	if( m_umlObjectMap.find(moving) == m_umlObjectMap.end() )
	{
		//kdDebug()<<"only uml objects may be moved around: -> "<<moving->text(0)<<endl;
		return false;
	}
	if( m_umlObjectMap.find(parent) == m_umlObjectMap.end() )
	{//parent is not a classifier (atts and ops are never returned by findDrop)
	 // so maybe  it's a folder.. check types
		if( parent->text(1) == "operations" && typeid(*(me->m_umlObjectMap[moving])) == typeid(UMLOperation))
		{
			//kdDebug()<<"moving operation to operations folder, OK"<<endl;
			return true;
		}
		if( parent->text(1) == "attributes" && typeid(*(me->m_umlObjectMap[moving])) == typeid(UMLAttribute))
		{
			//kdDebug()<<"moving attribute to attributes folder, OK"<<endl;
			return true;
		}
		//kdDebug()<<"moving to item "<<parent->text(0)<<" -- "<<parent->text(1)<<" not valid"<<endl;
		return false;
	}
	
	//we are moving a uml object (att or op) to another uml object (classifier)
	UMLObject *movingObject, *parentObject;
	movingObject = me->m_umlObjectMap[moving];
	parentObject = me->m_umlObjectMap[parent];
	//only move atts and ops
	if( (typeid(*movingObject) != typeid(UMLAttribute)) && (typeid(*movingObject) != typeid(UMLOperation)) )
	{
		//kdDebug()<<"only operations and attributes are movable!"<<endl;
		return false;
	}
	if( dynamic_cast<UMLClassifier*>(parentObject) == 0 )
	{
		//kdDebug()<<"new parent must be a classifier! (trying to move to "<<typeid(*parentObject).name()<<" )"<<endl;
		return false;
	}
	if( (typeid(*movingObject) == typeid(UMLAttribute)) && (typeid(*parentObject) != typeid(UMLClass)) )
	{
		//kdDebug()<<"attributes can only go into classes! (trying to move to "<<typeid(*par).name()<<" )"<<endl;
		return false;
	}
	return true;
	
// 	kdDebug()<<"moving item "<<moving->text(0)<<"and putint it into item"<<parent->text(0)
// 	<<"after"<<(afterme != 0 ?afterme->text(0):"noone")<<endl;
}


void RefactoringAssistant::movableDropEvent (QListViewItem* parentItem, QListViewItem* afterme)
{
//when dropping on a class, we have to put the item in the appropiate folder!
	UMLObject *movingObject, *parentObject, *oldParent;
	QListViewItem *movingItem;
	
	for( movingItem = firstChild(); movingItem != 0; movingItem = movingItem->itemBelow() )
	{
		if( movingItem->isSelected() )
			break;
	}
	if( !movingItem || (movingItem == afterme) || (m_umlObjectMap.find(movingItem) == m_umlObjectMap.end()) )
	{
		kdWarning()<<"Moving item not found or dropping after itself or item not found in uml obj map. aborting. (drop had already been accepted)"<<endl;
		return;
	}
	movingObject = m_umlObjectMap[movingItem];
	
	if( m_umlObjectMap.find(parentItem) == m_umlObjectMap.end() )
	{
		//we are dropping on a folder.. find the owner uml object
		QListViewItem *i = parentItem->parent();
		if( m_umlObjectMap.find(i) != m_umlObjectMap.end() )
		{
			parentObject = m_umlObjectMap[i];
		}
	}
	else
	{	//we are trying to drop on a classifier, reparent the item to the appropiate folder
		parentObject = m_umlObjectMap[parentItem];
		QString lookfor = (typeid(*movingObject) == typeid(UMLOperation) ? "operations" : "attributes" );
		kdDebug()<<"looking for folder "<<lookfor<<"in item "<<parentItem->text(0)<<endl;
		QListViewItem *newparent = parentItem->firstChild();
		for( ; newparent ; newparent = newparent->nextSibling() )
		{
			if( newparent->text(1) == lookfor )
				break;
		}
		if(!newparent)
		{
			kdWarning()<<"rejecting already accepted drop event because I dont know where to put the item"
				<<"moving "<<movingItem->text(0)<<" to "<<parentItem->text(0)<<endl;
			return;
		}
		parentItem = newparent;
		afterme = 0L;
	}
	
	//find old parent: movable items are always in folders, which are in uml objects
	QListViewItem *oldParentItem = movingItem->parent()->parent();
	if( m_umlObjectMap.find(oldParentItem) == m_umlObjectMap.end() )
	{
		kdWarning()<<"cannot find current parent of moving object. aborting"<<endl;
		return;
	}
	oldParent = m_umlObjectMap[oldParentItem];
	
	if( !movingObject || !parentObject || !oldParent )
	{
		return;
	}
	moveItem(movingItem, parentItem, afterme);
	kdDebug()<<"move uml object "<<movingObject->getName()<<" from "<<oldParent->getName()<<" to "<<parentObject->getName()<<endl;
	if( typeid(*movingObject) == typeid(UMLOperation) )
	{//kdDebug()<<"moving operation"<<endl;
		dynamic_cast<UMLClassifier*>(oldParent)->removeOperation( movingObject );
		//m_doc->removeUMLObject( movingObject );
		dynamic_cast<UMLClassifier*>(parentObject)->addOperation( dynamic_cast<UMLOperation*>(movingObject ) );
	}
	else if( typeid(*movingObject) == typeid(UMLAttribute) )
	{//kdDebug()<<"moving attribute"<<endl;
		static_cast<UMLClass*>(oldParent)->removeAttribute( movingObject );
		static_cast<UMLClass*>(parentObject)->addAttribute( static_cast<UMLAttribute*>(movingObject ) );
	}
	//emit moved(moving, afterFirst, afterme);
	emit moved();
}

void RefactoringAssistant::loadPixmaps()
{
	KStandardDirs *dirs = KGlobal::dirs();
	QString dataDir = dirs -> findResourceDir( "data", "umbrello/pics/public.png" );
	dataDir += "/umbrello/pics/";

	m_pixmaps.Public.load( dataDir + "public.png" );
	m_pixmaps.Protected.load( dataDir + "protected.png" );
	m_pixmaps.Private.load( dataDir + "private.png" );
	m_pixmaps.Generalization.load( dataDir + "generalization.xpm" );
	//FIXME!, find a good icon
	m_pixmaps.Subclass.load( dataDir + "uniassoc.xpm" );


}

}




 

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
#include "../dialogs/classpropdlg.h"
#include "../dialogs/umloperationdialog.h"
#include "../dialogs/umlattributedialog.h"

#include <qpoint.h>
#include <qpopupmenu.h>

#include <typeinfo>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

using std::type_info;


namespace Umbrello {


RefactoringAssistant::RefactoringAssistant( UMLDoc *doc, UMLClassifier *obj, QWidget *parent, const char *name ):
		KListView( parent, name ), m_doc( doc )
{
	loadPixmaps();

	setRootIsDecorated( true );
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

UMLObject* RefactoringAssistant::findUMLObject( const QListViewItem *item )
{
	QListViewItem *i = const_cast<QListViewItem*>(item);
	if( m_umlObjectMap.find(i) == m_umlObjectMap.end() )
	{
		kdWarning()<<"RefactoringAssistant::findUMLObject( QListViewItem *item )"
			   <<"item with text "<<item->text(0)<<"not found in uml map!"<<endl;
		return 0L;
	}
	return m_umlObjectMap[i];

}

QListViewItem* RefactoringAssistant::findListViewItem( const UMLObject *obj )
{
	UMLObjectMap::iterator it;
	for( it = m_umlObjectMap.begin() ; it != m_umlObjectMap.end() ; ++it )
		if( (*it).second == obj )
			return (*it).first;
	kdWarning()<<"RefactoringAssistant::findListViewItem( UMLObject *obj )"
			   <<"object id "<<obj->getID()<<"does not have s ListItem"<<endl;
	return 0L;
}


void RefactoringAssistant::itemExecuted( QListViewItem *item )
{
	UMLObject *o = findUMLObject( item );
	if(o) editProperties( );
}

void RefactoringAssistant::setVisibilityIcon( QListViewItem *item , const UMLObject *obj )
{
	switch(obj->getScope())
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

void RefactoringAssistant::umlObjectModified( const UMLObject *obj )
{
	if( !obj )
		obj = dynamic_cast<const UMLObject*>(sender());
	QListViewItem *item = findListViewItem( obj );
	if( !item )
		return;
	item->setText( 0, obj->getName() );
	if( typeid(*obj) == typeid(UMLOperation) ||
	    typeid(*obj) == typeid(UMLAttribute) )
	{
		setVisibilityIcon( item, obj );
	}
}

void RefactoringAssistant::operationAdded( UMLObject *obj )
{
	UMLOperation *op = dynamic_cast<UMLOperation*>(obj);
	if(!op)
	{
		kdWarning()<<"RefactoringAssistant::operationAdded( UMLObject *obj ) "
			   <<"called for a non-operation object."<<endl;
		return;
	}
	UMLClassifier *c = dynamic_cast<UMLClassifier*>(op->parent());
	if(!c)
	{
		kdWarning()<<"RefactoringAssistant::operationAdded( UMLObject *obj ) "
			   <<" - Parent of operation is not a classifier!"<<endl;
		return;
	}
	QListViewItem *item = findListViewItem( c );
	if( !item )
	{
		return;
	}
	for( QListViewItem *folder = item->firstChild(); folder; folder = folder->nextSibling() )
	{
		if( folder->text(1) == "operations" )
		{
			item = new KListViewItem( folder, op->getName() );
			m_umlObjectMap[item] = op;
			connect( op, SIGNAL( modified() ), this, SLOT( umlObjectModified() ) );
			setVisibilityIcon( item, op );
			break;
		}
	}	
}

void RefactoringAssistant::attributeAdded( UMLObject * )
{
	kdWarning()<<"not implemented"<<endl;
}

void RefactoringAssistant::editProperties( )
{
	QListViewItem *item = selectedItem();
	if( item )
	{
		UMLObject *o = findUMLObject( item );
		if( o ) editProperties( o );
	}
}

void RefactoringAssistant::editProperties( UMLObject *obj )
{
	KDialogBase *dia(0);
	if(typeid(*obj) == typeid(UMLClass) || typeid(*obj) == typeid(UMLInterface))
	{
		dia = new ClassPropDlg(this,obj,0,true);
	}
	else if(typeid(*obj) == typeid(UMLOperation))
	{
		dia = new UMLOperationDialog(this,static_cast<UMLOperation*>(obj));
	}
	else if(typeid(*obj) == typeid(UMLAttribute))
	{
		dia = new UMLAttributeDialog(this,static_cast<UMLAttribute*>(obj));
	}
	else
	{
		kdWarning()<<"RefactoringAssistant::editProperties( UMLObject *o ) caled for unknown type "<<typeid(*obj).name()<<endl;
		return;
	}
	if( dia && dia->exec() )
	{
		// need to update something?
	}
	delete dia;
}

void RefactoringAssistant::showContextMenu(KListView* ,QListViewItem *item, const QPoint &p)
{
	m_menu->clear();
	UMLObject *obj = findUMLObject( item );
	if(obj)
	{// Menu for UMLObjects
		if(typeid(*obj) == typeid(UMLClass))
		{
		m_menu->insertItem(i18n("Add Superclass"),this,SLOT(addSuperClassifier()));
		m_menu->insertItem(i18n("Add Derived Class"),this,SLOT(addDerivedClassifier()));
// 		m_menu->insertItem(i18n("Add Interface Implementation"),this,SLOT(addInterfaceImplementation()));
		m_menu->insertItem(i18n("Add Operation"),this,SLOT(addOperation()));
		m_menu->insertItem(i18n("Add Attribute"),this,SLOT(addAttribute()));
		}
		else if(typeid(*obj) == typeid(UMLInterface))
		{
		m_menu->insertItem(i18n("Add Superinterface"),this,SLOT(addSuperClassifier()));
		m_menu->insertItem(i18n("Add Derived Interface"),this,SLOT(addDerivedClassifier()));
// 		m_menu->insertItem(i18n("Add Interface Implementation"),this,SLOT(addInterfaceImplementation()));
		m_menu->insertItem(i18n("Add Operation"),this,SLOT(addOperation()));
		}
// 		else
// 		{
// 		kdDebug()<<"No context menu for objects of type "<<typeid(*obj).name()<<endl;
// 		return;
// 		}
		m_menu->insertSeparator();
		m_menu->insertItem(i18n("Properties"),this,SLOT(editProperties()));
	}
	else
	{//menu for other ViewItems
		if( item->text(1) == "operations" )
		{
			m_menu->insertItem(i18n("Add Operation"),this,SLOT(addOperation()));
		}
		else if( item->text(1) == "attributes" )
		{
			m_menu->insertItem(i18n("Add Attribute"),this,SLOT(addAttribute()));
		}
		else
		{
			kdWarning()<<"RefactoringAssistant::showContextMenu() "
				   <<"called for extraneous item"<<endl;
			return;
		}
	}
	m_menu->exec(p);
}

void RefactoringAssistant::addSuperClassifier()
{
	QListViewItem *item = selectedItem();
	if(!item)
	{
		kdWarning()<<"RefactoringAssistant::addSuperClassifier() "
			   <<"called with no item selected"<<endl;
		return;
	}
	UMLObject *obj = findUMLObject( item );
	if( !dynamic_cast<UMLClassifier*>(obj) )
	{
		kdWarning()<<"RefactoringAssistant::addSuperClassifier() "
			   <<"called for a non-classifier object"<<endl;
		return;
	}
	
	//classes have classes and interfaces interfaces as super/derived classifiers
	kdDebug()<<"creating super classifier"<<endl;
	UMLObject *super = m_doc->createUMLObject( typeid(*obj) );
	if(!super)
		return;
	kdDebug()<<"creating association"<<endl;
	m_doc->createUMLAssociation( obj, super, Uml::at_Generalization );
	//refresh, add classifier to assistant	
}

void RefactoringAssistant::addDerivedClassifier()
{
	kdWarning()<<"RefactoringAssistant::addDerivedClassifier()"
		   <<"not implemented... finish addSuperClassifier() first!!"<<endl;
	return;
// 	QListViewItem *item = selectedListViewItem( );
// 	UMLObject *obj = findUMLObject( item );
// 	if( !dynamic_cast<UMLClassifier*>(obj) )
// 		return;
// 	//classes have classes and interfaces interfaces as super/derived classifiers
// 	UMLObject *n = m_doc->createUMLObject( typeid(*obj) );
// 	if(!n)
// 		return;
// 	m_doc->createUMLAssociation( n, obj, Uml::at_Generalization );
	//refresh, add classifier to assistant	
}

void RefactoringAssistant::addInterfaceImplementation()
{
	kdWarning()<<"RefactoringAssistant::addInterfaceImplementation()"
		   <<"not implemented... finish addSuperClassifier() first!!"<<endl;
	return;
// 	QListViewItem *item = selectedListViewItem( );
// 	UMLObject *obj = findUMLObject( item );
// 	if( !dynamic_cast<UMLClassifier*>(obj) )
// 		return;
// 	UMLObject *n = m_doc->createUMLObject( typeid(UMLInterface) );
// 	if(!n)
// 		return;
// 	m_doc->createUMLAssociation( n, obj, Uml::at_Realization );
// 	//refresh, add classifier to assistant	
}

void RefactoringAssistant::addOperation()
{
	QListViewItem *item = selectedItem();
	if(!item)
	{
		kdWarning()<<"RefactoringAssistant::addOperation() "
			   <<"called with no item selected"<<endl;
		return;
	}
	UMLClassifier *c = dynamic_cast<UMLClassifier*>(findUMLObject( item ));
	if( !c )
		return;
	m_doc->createOperation( c );
}

void RefactoringAssistant::addAttribute()
{
	kdWarning()<<"RefactoringAssistant::addAttribute() - not implemented"<<endl;
// 	QListViewItem *item = selectedListViewItem( );
// 	UMLClass *c = dynamic_cast<UMLClass*>(findUMLObject( item ));
// 	if( !c )
// 		return;
// 	m_doc->createAttribute( c );
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

	connect( classifier, SIGNAL(operationAdded(UMLObject*)),
		this,SLOT( operationAdded(UMLObject*)));
	connect( classifier, SIGNAL( modified() ), this, SLOT( umlObjectModified() ) );
		
	//only classes have attributes, interfaces only have operations
	if(typeid(*classifier) == typeid (UMLClass))
	{	//column 0 = visible string, column 1 = type (hidden)
		attsFolder = new KListViewItem( classifierItem, i18n("Attributes"), "attributes" );
		attsFolder->setPixmap(0,SmallIcon("folder_green_open"));
		attsFolder->setExpandable( true );
		UMLAttributeList* atts = static_cast<UMLClass*>(classifier)->getFilteredAttributeList();
		for( UMLAttribute *att = atts->first(); att ; att = atts->next() )
		{
			item = new KListViewItem( attsFolder, att->getName() );
			setVisibilityIcon( item, att );
			m_umlObjectMap[item] = att;
		}

	} else {
		kdWarning() << "unknown typeid in addClassifier" << endl;
		attsFolder = 0;
	}

	// add operations
	opsFolder = new KListViewItem( classifierItem, i18n("Operations"), "operations" );
	opsFolder->setPixmap(0,SmallIcon("folder_blue_open"));
	attsFolder->setExpandable( true );
	UMLOperationList *ops = classifier->getFilteredOperationsList();
	for( UMLOperation *op = ops->first(); op ; op = ops->next() )
	{
		operationAdded( op );
	}

	//if add parents
	if(addSuper)
	{
	superFolder = new KListViewItem( classifierItem, i18n("Super") );
	superFolder->setExpandable( true );
	UMLClassifierList super = classifier->findSuperClassConcepts( m_doc );
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
	derivedFolder->setExpandable( true );
	UMLClassifierList derived = classifier->findSubClassConcepts ( m_doc );
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
		return;
	}
	//clear the map!
	addClassifier( obj, 0, true, true, true );
	QListViewItem *item = firstChild();
	item->setOpen(true);
	for( item = item->firstChild(); item ; item = item->nextSibling() )
		item->setOpen(true);
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
//when dropping on a class, we have to put the item in the appropriate folder!
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
		} else {
			kdWarning() << "uninitialised parentObject pointer" << endl;
			parentObject = 0;
		}
	}
	else
	{	//we are trying to drop on a classifier, reparent the item to the appropriate folder
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
	takeItem(movingItem);
	parentItem->insertItem(movingItem);
	parentItem->moveItem(afterme);
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

}  //namespace Umbrello





#include "refactoringassistant.moc"

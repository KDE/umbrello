/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// system includes
#include <cstdlib>

// qt/kde includes
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// app includes
#include "class.h"
#include "operation.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlobjectlist.h"
#include "umlview.h"

UMLListView* UMLListViewItem::s_pListView = 0;

UMLListViewItem::UMLListViewItem( UMLListView * parent, QString name,
				  Uml::ListView_Type t, UMLObject* o)
  : QListViewItem(parent, name) {
	init();
	s_pListView = parent;
	m_Type = t;
	m_pObject = o;
	if (o)
		m_nId = o->getID();
	setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Home ) );
	setText( name );
	setRenameEnabled( 0, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem::UMLListViewItem(UMLListView * parent)
  : QListViewItem(parent) {
	init();
	if (parent != NULL)
		s_pListView = parent;
	else
		kdDebug() << "UMLListViewItem constructor called with a NULL listview parent" << endl;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent)
  : QListViewItem(parent)  {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, QString name, Uml::ListView_Type t,UMLObject*o)
  : QListViewItem(parent, name) {
	if (s_pListView == NULL) {
		kdDebug() << "UMLListViewItem internal error 1: s_pListView is NULL" << endl;
		exit(1);
	}
	init();
	m_Type = t;
	m_pObject = o;
	if( !o ) {
		m_nId = Uml::id_None;
		updateFolder();
	} else {
		updateObject();
		m_nId = o->getID();
	}
	if( t == Uml::lvt_Logical_View || t == Uml::lvt_UseCase_View ||
	    t == Uml::lvt_Component_View || t == Uml::lvt_Deployment_View ||
	    t == Uml::lvt_Datatype_Folder)  {
		setRenameEnabled( 0, false );
	} else {
		setRenameEnabled( 0, true );
	}
	setText( name );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, QString name, Uml::ListView_Type t,Uml::IDType id)
  : QListViewItem(parent, name) {
	if (s_pListView == NULL) {
		kdDebug() << "UMLListViewItem internal error 2: s_pListView is NULL" << endl;
		exit(1);
	}
	init();
	m_Type = t;
	m_nId = id;
	switch (m_Type) {
		case Uml::lvt_Collaboration_Diagram:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram_Collaboration ) );
			break;
		case Uml::lvt_Class_Diagram:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram_Class ) );
			break;
		case Uml::lvt_State_Diagram:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram_State ) );
			break;
		case Uml::lvt_Activity_Diagram:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram_Activity ) );
			break;
		case Uml::lvt_Sequence_Diagram:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram_Sequence ) );
			break;
		case Uml::lvt_Component_Diagram:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram_Component ) );
			break;
		case Uml::lvt_Deployment_Diagram:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram_Deployment ) );
			break;
		case Uml::lvt_UseCase_Diagram:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram_Usecase ) );
			break;
		default:
			setPixmap(0, s_pListView->getPixmap( UMLListView::it_Diagram ) );
	}
	/*
		Constructor also used by folder so just make sure we don't need to
		to set pixmap to folder.  doesn't hurt diagrams.
	*/
	updateFolder();
	setText( name );
	setRenameEnabled( 0, true );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem::~UMLListViewItem() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListViewItem::init() {
	m_Type = Uml::lvt_Unknown;
	m_bCreating = false;
	m_pObject = NULL;
	m_nId = Uml::id_None;
	m_nChildren = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
Uml::ListView_Type UMLListViewItem::getType() const {
	return m_Type;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
Uml::IDType UMLListViewItem::getID() const {
	if (m_pObject)
		return m_pObject->getID();
	return m_nId;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListViewItem::setID(Uml::IDType id) {
	if (m_pObject) {
		Uml::IDType oid = m_pObject->getID();
		if (id != Uml::id_None && oid != id)
			kdDebug() << "UMLListViewItem::setID: new id " << ID2STR(id)
				  << " does not agree with object id " << ID2STR(oid)
				  << endl;
	}
	m_nId = id;
}

bool UMLListViewItem::isOwnParent(Uml::IDType listViewItemID) {
	QListViewItem *lvi = (QListViewItem*)s_pListView->findItem(listViewItemID);
	if (lvi == NULL) {
		kdError() << "UMLListViewItem::isOwnParent: ListView->findItem("
			  << ID2STR(listViewItemID) << ") returns NULL" << endl;
		return true;
	}
	for (QListViewItem *self = (QListViewItem*)this; self; self = self->parent()) {
		if (lvi == self)
			return true;
	}
	return false;
}

void UMLListViewItem::updateObject() {
	if( m_pObject == NULL )
		return;

	Uml::Scope scope = m_pObject->getScope();
	setText( m_pObject->getName() );

	switch( m_pObject->getBaseType() ) {
		case Uml::ot_Actor:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Actor ) );
			break;

		case Uml::ot_UseCase:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_UseCase ) );
			break;

		case Uml::ot_Class:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Class ) );
			break;

		case Uml::ot_Template:
			setPixmap( 0, s_pListView->getPixmap(UMLListView::it_Template) );
			break;

		case Uml::ot_Package:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Package ) );
			break;

		case Uml::ot_Component:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Component ) );
			break;

		case Uml::ot_Node:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Node ) );
			break;

		case Uml::ot_Artifact:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Artifact ) );
			break;

		case Uml::ot_Interface:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Interface ) );
			break;

		case Uml::ot_Datatype:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Datatype ) );
			break;

		case Uml::ot_Enum:
			setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Enum ) );
			break;

		case Uml::ot_Operation:
			if( scope == Uml::Public )
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Public_Method ) );
			else if( scope == Uml::Private )
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Private_Method ) );
			else
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Protected_Method ) );
			break;

		case Uml::ot_Attribute:
			if( scope == Uml::Public )
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Public_Attribute ) );
			else if( scope == Uml::Private )
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Private_Attribute ) );
			else
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Protected_Attribute ) );
			break;
		default:
			break;
	}//end switch
}

void UMLListViewItem::updateFolder() {
	switch( m_Type ) {
		case Uml::lvt_UseCase_View:
		case Uml::lvt_UseCase_Folder:
			if( isOpen() )
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Folder_Grey_Open ) );
			else
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Folder_Grey ) );
			break;

		case Uml::lvt_Logical_View:
		case Uml::lvt_Logical_Folder:
			if( isOpen() )
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Folder_Green_Open ) );
			else
				setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Folder_Green ) );
			break;

		case Uml::lvt_Datatype_Folder:
			if ( isOpen() )  {
				setPixmap( 0, s_pListView->getPixmap(UMLListView::it_Folder_Orange_Open) );
			} else {
				setPixmap( 0, s_pListView->getPixmap(UMLListView::it_Folder_Orange) );
			}
			break;

		case Uml::lvt_Component_View:
		case Uml::lvt_Component_Folder:
			if ( isOpen() ) {
				setPixmap(0, s_pListView->getPixmap(UMLListView::it_Folder_Red_Open) );
			} else {
				setPixmap(0, s_pListView->getPixmap(UMLListView::it_Folder_Red) );
			}
			break;

		case Uml::lvt_Deployment_View:
		case Uml::lvt_Deployment_Folder:
			if ( isOpen() ) {
				setPixmap(0, s_pListView->getPixmap(UMLListView::it_Folder_Violet_Open) );
			} else {
				setPixmap(0, s_pListView->getPixmap(UMLListView::it_Folder_Violet) );
			}
			break;

		default:
			break;
	}
}

void UMLListViewItem::setOpen( bool open ) {
	QListViewItem::setOpen( open );
	updateFolder();
}

void UMLListViewItem::setText(QString newText) {
	m_Label = newText;
	QListViewItem::setText(0, newText);
}

QString UMLListViewItem::getText() const {
	return m_Label;
}

void UMLListViewItem::okRename( int col ) {
	QListViewItem::okRename( col );
	if (m_bCreating) {
		m_bCreating = false;
		if ( s_pListView -> slotItemRenamed( this, col ) ) {
			m_Label = text(col);
		}
		return;
	}
	QString newText = text( col );
        if ( newText == m_Label )
            return;
	UMLDoc * doc = s_pListView -> getDocument();
	if( newText.isEmpty() ) {
		KMessageBox::error( kapp->mainWidget() ,
				    i18n("The name you entered was invalid.\nRenaming process has been canceled."),
				    i18n("Name Not Valid") );
		setText( m_Label );
		return;
	}
	switch( m_Type ) {
		case Uml::lvt_UseCase:
		case Uml::lvt_Actor:
		case Uml::lvt_Class:
		case Uml::lvt_Package:
		case Uml::lvt_Interface:
		case Uml::lvt_Datatype:
		case Uml::lvt_Enum:
			if (m_pObject == NULL || !doc->isUnique(newText)) {
				cancelRenameWithMsg();
				return;
			}
			m_pObject -> setName( newText );
			m_Label = newText;
			break;

		case Uml::lvt_Operation:
		{
			if (m_pObject == NULL) {
				cancelRenameWithMsg();
				return;
			}
			UMLOperation *op = static_cast<UMLOperation*>(m_pObject);
			UMLClassifier *parent = static_cast<UMLClassifier *>( op -> parent() );
			//see if op already has that name and not the op/att we are renaming
			//then give a warning about the name being the same
			UMLObjectList list = parent -> findChildObject( op->getBaseType(), newText );
			if(list.isEmpty() || (!list.isEmpty() && KMessageBox::warningYesNo( kapp -> mainWidget() ,
					      i18n( "The name you entered was not unique.\nIs this what you wanted?" ),
					      i18n( "Name Not Unique" ) ) == KMessageBox::Yes )) {
				op->setName( newText );
				m_Label = newText;
			} else {
				QListViewItem::setText(0, m_Label);
			}
			break;
		}

		case Uml::lvt_Attribute:
		{
			if (m_pObject == NULL) {
				cancelRenameWithMsg();
				return;
			}
			UMLClass *parent = static_cast<UMLClass*>( m_pObject -> parent() );
			UMLObjectList list = parent -> findChildObject( m_pObject -> getBaseType(), newText );
			if (! list.isEmpty()) {
				cancelRenameWithMsg();
				return;
			}
			m_pObject -> setName( newText );
			m_Label = newText;
			break;
		}

		case Uml::lvt_UseCase_Diagram:
		case Uml::lvt_Class_Diagram:
		case Uml::lvt_Sequence_Diagram:
		case Uml::lvt_Collaboration_Diagram:
		case Uml::lvt_State_Diagram:
		case Uml::lvt_Activity_Diagram:
		case Uml::lvt_Component_Diagram:
		case Uml::lvt_Deployment_Diagram:
		{
			UMLView *view = doc -> findView( getID() );
			if (view == NULL) {
				cancelRenameWithMsg();
				return;
			}
			UMLView *anotherView = doc -> findView( view->getType(), newText );
			if( anotherView && anotherView -> getID() == getID() )
				anotherView = 0;
			if (anotherView) {
				cancelRenameWithMsg();
				return;
			}
			view->setName( newText );
			m_Label = newText;
			doc->signalDiagramRenamed(view);
			break;
		}
		case Uml::lvt_UseCase_Folder:
		case Uml::lvt_Logical_Folder:
		case Uml::lvt_Component_Folder:
		case Uml::lvt_Deployment_Folder:
			m_Label = newText;
			break;
		default:
			cancelRenameWithMsg();
			break;
	}
}

void UMLListViewItem::cancelRenameWithMsg() {
	KMessageBox::error( kapp->mainWidget() ,
			    i18n("The name you entered was invalid.\nRenaming process has been canceled."),
			    i18n("Name Not Valid") );
	QListViewItem::setText(0, m_Label);
}

void UMLListViewItem::cancelRename(int col) {
	QListViewItem::cancelRename(col);
	if (m_bCreating) {
		s_pListView->cancelRename(this);
	}
}

// sort the listview items by type and alphabetically
int UMLListViewItem::compare(QListViewItem *other, int col, bool ascending) const
{
	Uml::ListView_Type ourType = getType();
	Uml::ListView_Type otherType = static_cast<UMLListViewItem*>( other )->getType();

	if ( ourType == otherType )
		return key( col, ascending ).compare( other->key( col, ascending) );

	if ( ourType < otherType )
		return -1;
	if ( ourType > otherType )
		return 1;

	return 0;
}

UMLListViewItem* UMLListViewItem::deepCopy(UMLListViewItem *newParent) {
	QString nm = getText();
	Uml::ListView_Type t = getType();
	UMLObject *o = getUMLObject();
	UMLListViewItem* newItem;
	if (o)
		newItem = new UMLListViewItem(newParent, nm, t, o);
	else
		newItem = new UMLListViewItem(newParent, nm, t, m_nId);
	UMLListViewItem *childItem = static_cast<UMLListViewItem*>(firstChild());
	while (childItem) {
		childItem->deepCopy(newItem);
		childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
	}
	return newItem;
}

UMLListViewItem* UMLListViewItem::findUMLObject(UMLObject *o) {
	if (m_pObject == o)
		return this;
	UMLListViewItem *childItem = static_cast<UMLListViewItem*>(firstChild());
	while (childItem) {
		UMLListViewItem *inner = childItem->findUMLObject(o);
		if (inner)
			return inner;
		childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
	}
	return NULL;
}

UMLListViewItem * UMLListViewItem::findItem(Uml::IDType id) {
	if (getID() == id)
		return this;
	UMLListViewItem *childItem = static_cast<UMLListViewItem*>(firstChild());
	while (childItem) {
		UMLListViewItem *inner = childItem->findItem(id);
		if (inner)
			return inner;
		childItem = static_cast<UMLListViewItem*>(childItem->nextSibling());
	}
	return NULL;
}

void UMLListViewItem::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement itemElement = qDoc.createElement( "listitem" );
	Uml::IDType id = getID();
	if (id != Uml::id_None)
		itemElement.setAttribute( "id", ID2STR(id) );
	itemElement.setAttribute( "type", m_Type );
	if (m_pObject == NULL) {
		// The predefined listview items such as "Logical View" etc. do
		// not have a model counterpart thus their label is saved here.
		itemElement.setAttribute( "label", m_Label );
	} else if (m_pObject->getID() == Uml::id_None) {
		if (m_Label.isEmpty()) {
			kdDebug() << "UMLListViewItem::saveToXMI(): Skipping empty item"
				  << endl;
			return;
		}
		kdDebug() << "UMLListViewItem::saveToXMI(): saving local label "
			  << m_Label << " because umlobject ID is not set" << endl;
		itemElement.setAttribute( "label", m_Label );
	}
	itemElement.setAttribute( "open", isOpen() );
	UMLListViewItem * childItem = static_cast<UMLListViewItem *> ( firstChild() );
	while( childItem ) {
		childItem -> saveToXMI( qDoc, itemElement );
		childItem = static_cast<UMLListViewItem *> ( childItem -> nextSibling() );
	}
	qElement.appendChild( itemElement );
}

bool UMLListViewItem::loadFromXMI(QDomElement& qElement) {
	QString id = qElement.attribute( "id", "-1" );
	QString type = qElement.attribute( "type", "-1" );
	QString label = qElement.attribute( "label", "" );
	QString open = qElement.attribute( "open", "1" );
	if (!label.isEmpty())
		setText( label );
	else if (id == "-1") {
		kdError() << "UMLListViewItem::loadFromXMI: Item of type "
			  << type << " has neither ID nor label" << endl;
		return false;
	}

	m_nChildren = qElement.childNodes().count();

	m_nId = STR2ID(id);
	if (m_nId != Uml::id_None)
		m_pObject = s_pListView->getDocument()->findObjectById( m_nId );
	m_Type = (Uml::ListView_Type)(type.toInt());
	setOpen( (bool)open.toInt() );
	return true;
}


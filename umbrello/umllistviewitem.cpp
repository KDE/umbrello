/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "class.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlobjectlist.h"
#include "umlview.h"
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

UMLListView* UMLListViewItem::s_pListView = 0;

UMLListViewItem::UMLListViewItem( UMLListView * parent, QString name,
                                  Uml::ListView_Type t, UMLObject* o)
  : QListViewItem(parent, name) {
	m_bCreating = false;
	s_pListView = parent;
	m_Type = t ;
	m_pObject = o;
	if (o)
		m_nId = o->getID();
	else
		m_nId = -1;
	setPixmap( 0, s_pListView -> getPixmap( UMLListView::it_Home ) );
	setText( name );
	setRenameEnabled( 0, false );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem::UMLListViewItem(UMLListView * parent)
  : QListViewItem(parent) {
	m_bCreating = false;
	if (s_pListView == NULL) {
		s_pListView = parent;
	}
	if (parent == NULL)
		kdDebug() << "UMLListViewItem constructor called with a NULL parent" << endl;
	m_Type = Uml::lvt_Unknown;
	m_pObject = NULL;
	m_nId = -1;
	m_nChildren = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, QString name, Uml::ListView_Type t,UMLObject*o)
  : QListViewItem(parent, name) {
	m_bCreating = false;
	if (s_pListView == NULL) {
		kdDebug() << "UMLListViewItem internal error 1: s_pListView is NULL" << endl;
		exit(1);
	}
	m_Type = t;
	m_pObject = o;
	if( !o ) {
		m_nId = -1;
		updateFolder();
	} else {
		updateObject();
		m_nId = o->getID();
	}
	if( t == Uml::lvt_Logical_View || t == Uml::lvt_UseCase_View ||
	    t == Uml::lvt_Component_View || t == Uml::lvt_Deployment_View )
		setRenameEnabled( 0, false );
	else
		setRenameEnabled( 0, true );
	setText( name );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, QString name, Uml::ListView_Type t,int id)
  : QListViewItem(parent, name) {
	m_bCreating = false;
	if (s_pListView == NULL) {
		kdDebug() << "UMLListViewItem internal error 2: s_pListView is NULL" << endl;
		exit(1);
	}
	m_Type = t;
	m_pObject = NULL;
	m_nId = id;
	setPixmap(0, s_pListView -> getPixmap( UMLListView::it_Diagram ) );
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
Uml::ListView_Type UMLListViewItem::getType() const {
	return m_Type;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLListViewItem::getID() const {
	if (m_pObject)
		return m_pObject->getID();
	return m_nId;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLListViewItem::setID(int id) {
	if (m_pObject) {
		int oid = m_pObject->getID();
		if (id != -1 && oid != id)
			kdDebug() << "UMLListViewItem::setID: new id " << id
				  << " does not agree with object id " << oid << endl;
	}
	m_nId = id;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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

void UMLListViewItem::okRename( int col ) {
	QListViewItem::okRename( col );
	if (m_bCreating) {
		m_bCreating = false;
		if ( s_pListView -> slotItemRenamed( this, 0 ) ) {
			m_Label = text(col);
		} else {
			startRename(0);
		}
		return;
	}
	QString newText = text( col );
	UMLObject * object = 0;
	UMLClassifier * parent = 0;
	UMLView * view = 0, * anotherView;
	UMLDoc * doc = s_pListView -> getDocument();
	if( newText.length() == 0 ) {
		KMessageBox::error( kapp->mainWidget() ,
				    i18n("The name you entered was invalid!\nRenaming process has been canceled."),
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
			object = m_pObject;
			if( object ) {
				object = doc -> findUMLObject( object -> getBaseType(), newText );
				if( object && object == m_pObject )
					object = 0;
				if( !object ) {
					m_pObject -> setName( newText );
					m_Label = newText;
					return;
				}
			}
			break;

		case Uml::lvt_Operation:
			object = m_pObject;
			if( object ) {
				parent = static_cast<UMLClassifier *>( object -> parent() );
				//see if op already has that name and not the op/att we are renaming
				//then give a warning about the name being the same
				UMLObjectList list = parent -> findChildObject( object -> getBaseType(), newText );
				if(list.isEmpty() || (!list.isEmpty() && KMessageBox::warningYesNo( kapp -> mainWidget() ,
				                      i18n( "The name you entered was not unique!\nIs this what you wanted?" ),
				                      i18n( "Name Not Unique" ) ) == KMessageBox::Yes )) {
					object -> setName( newText );
					m_Label = newText;
					return;
				}
				setText( m_Label );
				return;
			}
			break;

		case Uml::lvt_Attribute:
			object = m_pObject;
			if( object ) {
				parent = static_cast<UMLClass*>( object -> parent() );
				UMLObjectList list = parent -> findChildObject( object -> getBaseType(), newText );
				if (list.isEmpty()) {
					object -> setName( newText );
					m_Label = newText;
					return;
				}
			}
			break;

		case Uml::lvt_UseCase_Diagram:
		case Uml::lvt_Class_Diagram:
		case Uml::lvt_Sequence_Diagram:
		case Uml::lvt_Collaboration_Diagram:
		case Uml::lvt_State_Diagram:
		case Uml::lvt_Activity_Diagram:
		case Uml::lvt_Component_Diagram:
		case Uml::lvt_Deployment_Diagram:
			view = doc -> findView( getID() );
			if( view ) {
				anotherView = doc -> findView( view -> getType(), newText );
				if( anotherView && anotherView -> getID() == getID() )
					anotherView = 0;
				if( !anotherView ) {
					view->setName( newText );
					m_Label = newText;
					doc->signalDiagramRenamed(view);
					return;
				}
			}
			break;
		case Uml::lvt_UseCase_Folder:
		case Uml::lvt_Logical_Folder:
		case Uml::lvt_Component_Folder:
		case Uml::lvt_Deployment_Folder:
			m_Label = newText;
			return;
			break;
		default:
			break;
	}
	KMessageBox::error( kapp->mainWidget() ,
			    i18n("The name you entered was invalid!\nRenaming process has been canceled."),
	                    i18n("Name Not Valid") );
	setText( m_Label );
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

bool UMLListViewItem::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement itemElement = qDoc.createElement( "listitem" );
	itemElement.setAttribute( "id", getID() );
	itemElement.setAttribute( "type", m_Type );
	itemElement.setAttribute( "label", m_Label );
	itemElement.setAttribute( "open", isOpen() );
	UMLListViewItem * childItem = static_cast<UMLListViewItem *> ( firstChild() );
	while( childItem ) {
		bool status = childItem -> saveToXMI( qDoc, itemElement );
		if( !status )
			return false;
		childItem = static_cast<UMLListViewItem *> ( childItem -> nextSibling() );
	}
	qElement.appendChild( itemElement );
	return true;
}

bool UMLListViewItem::loadFromXMI(QDomElement& qElement) {
	QString id = qElement.attribute( "id", "-1" );
	QString type = qElement.attribute( "type", "-1" );
	setText( qElement.attribute( "label", "" ) );
	QString open = qElement.attribute( "open", "1" );

	m_nChildren = qElement.childNodes().count();

	m_nId = id.toInt();
	if (m_nId != -1)
		m_pObject = s_pListView->getDocument()->findUMLObject( m_nId );
	m_Type = (Uml::ListView_Type)(type.toInt());
	setOpen( (bool)open.toInt() );
	return true;
}


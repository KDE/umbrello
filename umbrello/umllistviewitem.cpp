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

// system includes
#include <cstdlib>

// qt/kde includes
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

// app includes
#include "class.h"
#include "template.h"
#include "attribute.h"
#include "operation.h"
#include "umldoc.h"
#include "umllistview.h"
#include "umllistviewitem.h"
#include "umlobjectlist.h"
#include "umlview.h"
#include "model_utils.h"

UMLListView* UMLListViewItem::s_pListView = 0;

UMLListViewItem::UMLListViewItem( UMLListView * parent, const QString &name,
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
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, const QString &name, Uml::ListView_Type t,UMLObject*o)
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
UMLListViewItem::UMLListViewItem(UMLListViewItem * parent, const QString &name, Uml::ListView_Type t,Uml::IDType id)
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
	Uml::Object_Type ot = m_pObject->getBaseType();
	QString modelObjText = m_pObject->getName();
	if (ot == Uml::ot_Operation || ot == Uml::ot_Attribute || ot == Uml::ot_Template) {
		UMLClassifierListItem *pNarrowed = static_cast<UMLClassifierListItem*>(m_pObject);
		modelObjText = pNarrowed->toString(Uml::st_SigNoScope);
	}
	setText(modelObjText);

	UMLListView::Icon_Type icon = UMLListView::it_Home;
	switch (ot) {
		case Uml::ot_Actor:
			icon = UMLListView::it_Actor;
			break;

		case Uml::ot_UseCase:
			icon = UMLListView::it_UseCase;
			break;

		case Uml::ot_Class:
			icon = UMLListView::it_Class;
			break;

		case Uml::ot_Template:
			icon = UMLListView::it_Template;
			break;

		case Uml::ot_Package:
			icon =  UMLListView::it_Package;
			break;

		case Uml::ot_Component:
			icon =  UMLListView::it_Component;
			break;

		case Uml::ot_Node:
			icon =  UMLListView::it_Node;
			break;

		case Uml::ot_Artifact:
			icon =  UMLListView::it_Artifact;
			break;

		case Uml::ot_Interface:
			icon =  UMLListView::it_Interface;
			break;

		case Uml::ot_Datatype:
			icon =  UMLListView::it_Datatype;
			break;

		case Uml::ot_Enum:
			icon =  UMLListView::it_Enum;
			break;

		case Uml::ot_Entity:
			icon =  UMLListView::it_Entity;
			break;

		case Uml::ot_Operation:
			if( scope == Uml::Public )
				icon =  UMLListView::it_Public_Method;
			else if( scope == Uml::Private )
				icon =  UMLListView::it_Private_Method;
			else
				icon =  UMLListView::it_Protected_Method;
			break;

		case Uml::ot_Attribute:
			if( scope == Uml::Public )
				icon =  UMLListView::it_Public_Attribute;
			else if( scope == Uml::Private )
				icon =  UMLListView::it_Private_Attribute;
			else
				icon =  UMLListView::it_Protected_Attribute;
			break;
		default:
			break;
	}//end switch
	if (icon)
		setPixmap(0, s_pListView->getPixmap(icon) );
}

void UMLListViewItem::updateFolder() {
	UMLListView::Icon_Type icon = UMLListView::it_Home;
	switch( m_Type ) {
		case Uml::lvt_UseCase_View:
		case Uml::lvt_UseCase_Folder:
			icon = UMLListView::it_Folder_Grey;
			break;

		case Uml::lvt_Logical_View:
		case Uml::lvt_Logical_Folder:
			icon = UMLListView::it_Folder_Green;
			break;

		case Uml::lvt_Datatype_Folder:
			icon = UMLListView::it_Folder_Orange;
			break;

		case Uml::lvt_Component_View:
		case Uml::lvt_Component_Folder:
			icon = UMLListView::it_Folder_Red;
			break;

		case Uml::lvt_Deployment_View:
		case Uml::lvt_Deployment_Folder:
			icon = UMLListView::it_Folder_Violet;
			break;

		case Uml::lvt_EntityRelationship_Model:
		case Uml::lvt_EntityRelationship_Folder:
			icon = UMLListView::it_Folder_Cyan;
			break;

		default:
			break;
	}
	if (icon) {
		icon = (UMLListView::Icon_Type)((int)icon + (int)isOpen());
		setPixmap(0, s_pListView->getPixmap(icon) );
	}
}

void UMLListViewItem::setOpen( bool open ) {
	QListViewItem::setOpen( open );
	updateFolder();
}

void UMLListViewItem::setText(const QString &newText) {
	m_Label = newText;
	QListViewItem::setText(0, newText);
}

QString UMLListViewItem::getText() const {
	return m_Label;
}

void UMLListViewItem::okRename( int col ) {
	QListViewItem::okRename( col );
	UMLDoc* doc = s_pListView->getDocument();
	if (m_bCreating) {
		m_bCreating = false;
		if ( s_pListView->slotItemRenamed( this, col ) ) {
			m_Label = text(col);
			doc->setModified(true);
		}
		return;
	}
	QString newText = text( col );
        if ( newText == m_Label ) {
            return;
        }
	if( newText.isEmpty() ) {
		cancelRenameWithMsg();
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
			doc->setModified(true);
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
			Umbrello::OpDescriptor od;
			Umbrello::Parse_Status st = Umbrello::parseOperation(newText, od, parent);
			if (st == Umbrello::PS_OK) {
			// TODO: Check that no operation with the exact same profile exists.
				op->setName( od.m_name );
				op->setType( od.m_pReturnType );
				UMLAttributeList* parmList = op->getParmList();
				unsigned i = 0;
				if (parmList->count() > od.m_args.count()) {
					for (i = od.m_args.count(); i < parmList->count(); i++) {
						parmList->remove(i);
					}
					i = 0;
				}
				for (Umbrello::NameAndType_ListIt lit = od.m_args.begin();
				     lit != od.m_args.end(); ++lit, ++i) {
					const Umbrello::NameAndType& nm_tp = *lit;
					UMLAttribute *a;
					if (i < parmList->count()) {
						a = parmList->at(i);
						a->setName(nm_tp.m_name);
						a->setType(nm_tp.m_type);
						a->setInitialValue(nm_tp.m_initialValue);
					} else {
						a = new UMLAttribute(op);
						a->setID( doc->getUniqueID() );
						a->setName(nm_tp.m_name);
						a->setType(nm_tp.m_type);
						a->setInitialValue(nm_tp.m_initialValue);
						op->addParm(a);
					}
				}
				m_Label = op->toString(Uml::st_SigNoScope);
			} else {
				KMessageBox::error( kapp->mainWidget(),
						    Umbrello::psText(st),
						    i18n("Rename canceled") );
			}
			QListViewItem::setText(0, m_Label);
			break;
		}

		case Uml::lvt_Attribute:
		{
			if (m_pObject == NULL) {
				cancelRenameWithMsg();
				return;
			}
			UMLClass *parent = static_cast<UMLClass*>( m_pObject -> parent() );
			Umbrello::NameAndType nt;
			Umbrello::Parse_Status st = Umbrello::parseAttribute(newText, nt, parent);
			if (st == Umbrello::PS_OK) {
				UMLObjectList list = parent->findChildObject( m_pObject->getBaseType(), newText );
				if (! list.isEmpty()) {
					cancelRenameWithMsg();
					return;
				}
				m_pObject->setName(nt.m_name);
				UMLAttribute *pAtt = static_cast<UMLAttribute*>(m_pObject);
				pAtt->setType(nt.m_type);
				pAtt->setInitialValue(nt.m_initialValue);
				m_Label = pAtt->toString(Uml::st_SigNoScope);
			} else {
				KMessageBox::error( kapp->mainWidget(),
						    Umbrello::psText(st),
						    i18n("Rename canceled") );
			}
			QListViewItem::setText(0, m_Label);
			break;
		}

		case Uml::lvt_Template:
		{
			if (m_pObject == NULL) {
				cancelRenameWithMsg();
				return;
			}
			UMLClassifier *parent = static_cast<UMLClassifier*>(m_pObject->parent());
			Umbrello::NameAndType nt;
			Umbrello::Parse_Status st = Umbrello::parseTemplate(newText, nt, parent);
			if (st == Umbrello::PS_OK) {
				UMLObjectList list = parent->findChildObject( m_pObject->getBaseType(), newText );
				if (! list.isEmpty()) {
					cancelRenameWithMsg();
					return;
				}
				m_pObject->setName(nt.m_name);
				UMLTemplate *tmpl = static_cast<UMLTemplate*>(m_pObject);
				tmpl->setType(nt.m_type);
				m_Label = tmpl->toString(Uml::st_SigNoScope);
			} else {
				KMessageBox::error( kapp->mainWidget(),
						    Umbrello::psText(st),
						    i18n("Rename canceled") );
			}
			QListViewItem::setText(0, m_Label);
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
			KMessageBox::error( kapp->mainWidget() ,
			    i18n("Renaming an item of listview type %1 is not yet implemented.").arg(m_Type),
			    i18n("Function Not Implemented") );
			QListViewItem::setText(0, m_Label);
			break;
	}
	doc->setModified(true);
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
	UMLListViewItem *ulvi = static_cast<UMLListViewItem*>(other);
	Uml::ListView_Type ourType = getType();
	Uml::ListView_Type otherType = ulvi->getType();

	if ( ourType == otherType ) {
		UMLObject *otherObj = ulvi->getUMLObject();
		if (m_pObject == NULL || otherObj == NULL)
			return key( col, ascending ).compare( other->key( col, ascending) );
		if (ourType == Uml::lvt_Attribute) {
			UMLClass *ourParent = dynamic_cast<UMLClass*>(m_pObject->parent());
			UMLClass *otherParent = dynamic_cast<UMLClass*>(otherObj->parent());
			if (ourParent == NULL || otherParent == NULL || ourParent != otherParent) {
				kdError() << "UMLListViewItem::compare(UMLAttribute): ourParent="
					  << ourParent << ", otherParent=" << otherParent << endl;
				return key( col, ascending ).compare( other->key( col, ascending) );
			}
			UMLAttributeList atts = ourParent->getAttributeList();
			int myIndex = atts.findRef( static_cast<UMLAttribute*>(m_pObject) );
			int otherIndex = atts.findRef( static_cast<UMLAttribute*>(otherObj) );
			return (myIndex < otherIndex ? -1 : myIndex > otherIndex ? 1 : 0);
		} else if (ourType == Uml::lvt_Operation) {
			UMLClassifier *ourParent = dynamic_cast<UMLClassifier*>(m_pObject->parent());
			UMLClassifier *otherParent = dynamic_cast<UMLClassifier*>(otherObj->parent());
			if (ourParent == NULL || otherParent == NULL || ourParent != otherParent) {
				kdError() << "UMLListViewItem::compare(UMLOperation): ourParent="
					  << ourParent << ", otherParent=" << otherParent << endl;
				return key( col, ascending ).compare( other->key( col, ascending) );
			}
			UMLOperationList ops = ourParent->getOpList();
			int myIndex = ops.findRef( static_cast<UMLOperation*>(m_pObject) );
			int otherIndex = ops.findRef( static_cast<UMLOperation*>(otherObj) );
			return (myIndex < otherIndex ? -1 : myIndex > otherIndex ? 1 : 0);
		} else {
			return key( col, ascending ).compare( other->key( col, ascending) );
		}
	}
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
	QString idStr = ID2STR(id);
	//kdDebug() << "UMLListViewItem::saveToXMI: id = " << idStr
	//	  << ", type = " << m_Type << endl;
	if (id != Uml::id_None)
		itemElement.setAttribute( "id", idStr );
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
		childItem = dynamic_cast<UMLListViewItem *> ( childItem -> nextSibling() );
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
	if (m_pObject)
		updateObject();
	setOpen( (bool)open.toInt() );
	return true;
}


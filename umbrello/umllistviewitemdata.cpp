/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatastream.h>
#include "umllistviewitemdata.h"
#include "umllistviewitem.h"
#include "umlobject.h"
#include <kdebug.h>

UMLListViewItemData::UMLListViewItemData() {
	m_pItem = 0;
	object = 0;
	m_nChildren = 0;
}

UMLListViewItemData::UMLListViewItemData(UMLListViewItemData& Other) {
	*this = Other;
}

UMLListViewItemData::~UMLListViewItemData() {}

void UMLListViewItemData::setListViewItem(UMLListViewItem* Item) {
	m_pItem = Item;
}

UMLListViewItem* UMLListViewItemData::getListViewItem() {
	return m_pItem;
}

int UMLListViewItemData::childCount() {
	if(m_pItem) {
		m_nChildren = m_pItem->childCount();

		return m_nChildren;
	}

	return m_nChildren;
}

long UMLListViewItemData::getClipSizeOf() {
	long l_size = sizeof(m_nId) + sizeof(int) + sizeof(int);
	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string
	QString name = (QString)text(0);

	if (!name.length()) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{
		l_size += sizeof(tmp);
	} else {
		l_size += (name.length()*sizeof(QChar));
	}
	l_size += sizeof(Q_INT32);//open status

	return l_size;
}

bool UMLListViewItemData::clipSerialize(QDataStream *s, const bool toClip) {
	int open = 1;
	if( m_pItem )
		open = (int)m_pItem -> isOpen();
	if(toClip) {
		int count = childCount();
		int t = m_Type;
		*s << m_nId
		<< t
		<< text(0)
		<< count
		<<  open;
	} else {
		int t;
		*s >> m_nId
		>> t
		>> label
		>> m_nChildren
		>> open;
		m_Type = (Uml::ListView_Type)t;
		if( m_pItem )
			m_pItem -> setOpen( (bool) open );
	}

	return true;
}

bool UMLListViewItemData::serialize(QDataStream *s, bool archive, int /* fileversion */)
{
	int open = 1;
	if( m_pItem )
		open = (int)m_pItem -> isOpen();
	if(archive) {
		int t = m_Type;
		int count = childCount();
		*s << m_nId
		<< t
		<< text(0)
		<< count
		<< open;
	} else {

		int t;
		*s >> m_nId
		>> t
		>> label
		>> m_nChildren
		>> open;
		m_Type = (Uml::ListView_Type)t;
		if( m_pItem )
			m_pItem -> setOpen( (bool)open );
	}

	return false;
}

QString UMLListViewItemData::text(int index) {
	if(m_pItem) {
		label = m_pItem->text(index);
		return label;
	}
	return label;
}

UMLListViewItemData& UMLListViewItemData::operator=(UMLListViewItemData & Other) {
	m_pItem = Other.m_pItem;
	object = Other.object;
	m_nId = Other.m_nId;
	m_Type = Other.m_Type;
	m_nChildren = Other.childCount();
	label = Other.text(0);

	return *this;
}

bool UMLListViewItemData::operator==(const UMLListViewItemData& Other) {
	if(m_pItem != Other.m_pItem) {
		return false;
	}
	if(!(object == Other.object)) /*It should be compared using UMLObject::operator==*/
	{
		return false;
	}
	if(m_nId != Other.m_nId) {
		return false;
	}
	if(m_Type != Other.m_Type) {
		return false;
	}
	if(m_nChildren != Other.m_nChildren) {
		return false;
	}
	if(label != Other.label) {
		return false;
	}

	return true;
}

bool UMLListViewItemData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement itemElement = qDoc.createElement( "listitem" );
	itemElement.setAttribute( "id", m_nId );
	itemElement.setAttribute( "type", m_Type );
	itemElement.setAttribute( "label", label );
	itemElement.setAttribute( "open", m_pItem -> isOpen() );
	UMLListViewItem * childItem = static_cast<UMLListViewItem *> ( m_pItem -> firstChild() );
	while( childItem ) {
		bool status = childItem -> getdata() -> saveToXMI( qDoc, itemElement );
		if( !status )
			return false;
		childItem = static_cast<UMLListViewItem *> ( childItem -> nextSibling() );
	}
	qElement.appendChild( itemElement );
	return true;
}

bool UMLListViewItemData::loadFromXMI(QDomElement& qElement) {
	QString id = qElement.attribute( "id", "-1" );
	QString type = qElement.attribute( "type", "-1" );
	label = qElement.attribute( "label", "" );
	QString open = qElement.attribute( "open", "1" );

	m_nChildren = qElement.childNodes().count();

	m_nId = id.toInt();
	m_Type = (Uml::ListView_Type)(type.toInt());
	if( m_pItem ) {
		m_pItem->setOpen( (bool)open.toInt() );
	}
	return true;
}



/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatastream.h>
#include <kdebug.h>

#include "objectwidgetdata.h"

ObjectWidgetData::ObjectWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_Type = Uml::wt_Object;
	m_InstanceName = "";
	m_bMultipleInstance= false;
	m_nLocalID = -1;
	m_bDrawAsActor = false;
	m_bShowDeconstruction = false;
	m_nLineLength = 0;
}

ObjectWidgetData::ObjectWidgetData(ObjectWidgetData & Other) : UMLWidgetData(Other) {
	*this = Other;
}

ObjectWidgetData::~ObjectWidgetData() {
}

ObjectWidgetData & ObjectWidgetData::operator=(ObjectWidgetData & Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;
	m_InstanceName = Other.m_InstanceName;
	m_bMultipleInstance= Other.m_bMultipleInstance;
	m_nLocalID = Other.m_nLocalID;
	m_bDrawAsActor = Other.m_bDrawAsActor;
	m_bShowDeconstruction = Other.m_bShowDeconstruction;
	return *this;
}

bool ObjectWidgetData::operator==(ObjectWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}
	if(m_InstanceName != Other.m_InstanceName) {
		return false;
	}
	if(m_bMultipleInstance != Other.m_bMultipleInstance) {
		return false;
	}
	if(m_nLocalID != Other.m_nLocalID) {
		return false;
	}
	if( m_bDrawAsActor != Other.m_bDrawAsActor )
		return false;
	if( m_bShowDeconstruction != Other.m_bShowDeconstruction )
		return false;

	return true;
}

QString ObjectWidgetData::getInstanceName() {
	return m_InstanceName;
}

void ObjectWidgetData::setInstanceName( QString InstanceName) {
	m_InstanceName = InstanceName;
}

int ObjectWidgetData::getLocalID() {
	return m_nLocalID;
}

void ObjectWidgetData::setLocalID( int LocalID) {
	m_nLocalID = LocalID;
}

bool ObjectWidgetData::getMultipleInstance() {
	return m_bMultipleInstance;
}

void ObjectWidgetData::setMultipleInstance( bool MultipleInstance) {
	m_bMultipleInstance = MultipleInstance;
}

long ObjectWidgetData::getClipSizeOf() {

	long l_size = UMLWidgetData::getClipSizeOf();
	l_size += sizeof(m_nLocalID);
	l_size += sizeof( int ); //m_bMultipleInstance
	l_size += sizeof( int ); //m_bDrawAsActor
	l_size += sizeof( int ); //m_bShowDeconstruction
	l_size += sizeof( int ); //m_nLineLength
	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	if ( !m_InstanceName.length() ) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{

		l_size += sizeof(tmp);
	} else {
		l_size += (m_InstanceName.length()*sizeof(QChar));
	}

	return l_size;
}

bool ObjectWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	if(!UMLWidgetData::serialize(s, archive, fileversion)) {
		return false;
	}
	if(archive) {
		*s << m_nLocalID
		<< m_InstanceName
		<< (int)m_bMultipleInstance
		<< (int)m_bDrawAsActor
		<< (int)m_bShowDeconstruction
		<< m_nLineLength;
	} else {
		int multi, actor, decon;
		*s >> m_nLocalID
		>> m_InstanceName
		>> multi;
		m_bMultipleInstance = multi;
		if (fileversion > 4)
		{
			*s >> actor
			>> decon
			>> m_nLineLength;
			m_bDrawAsActor = actor;
			m_bShowDeconstruction = decon;
		}
		else
		{
			m_bDrawAsActor = false;
			m_bShowDeconstruction = false;
			m_nLineLength = 200;
		}
	}

	return true;
}

void ObjectWidgetData::print2cerr() {
	UMLWidgetData::print2cerr();
	kdDebug() << "m_InstanceName = " << m_InstanceName << endl;
	if(m_bMultipleInstance) {
		kdDebug() << "m_bMultipleInstance = " << "true" << endl;
	} else {
		kdDebug() << "m_bMultipleInstance = " << "false" << endl;
	}
	kdDebug() << "m_nLocalID = " << m_nLocalID << endl;
}

bool ObjectWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement objectElement = qDoc.createElement( "UML:ObjectWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, objectElement );
	objectElement.setAttribute( "instancename", m_InstanceName );
	objectElement.setAttribute( "drawasactor", m_bDrawAsActor );
	objectElement.setAttribute( "multipleinstance", m_bMultipleInstance );
	objectElement.setAttribute( "localid", m_nLocalID );
	objectElement.setAttribute( "decon", m_bShowDeconstruction );
	objectElement.setAttribute( "length", m_nLineLength );
	qElement.appendChild( objectElement );
	return status;
}

bool ObjectWidgetData::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidgetData::loadFromXMI( qElement ) )
		return false;
	m_InstanceName = qElement.attribute( "instancename", "" );
	QString draw = qElement.attribute( "drawasactor", "0" );
	QString multi = qElement.attribute( "multipleinstance", "0" );
	QString localid = qElement.attribute( "localid", "0" );
	QString decon = qElement.attribute( "decon", "0" );
	QString length = qElement.attribute( "length", "0" );

	m_bDrawAsActor = (bool)draw.toInt();
	m_bMultipleInstance = (bool)multi.toInt();
	m_nLocalID = localid.toInt();
	m_bShowDeconstruction = (bool)decon.toInt();
	m_nLineLength = length.toInt();
	return true;

}





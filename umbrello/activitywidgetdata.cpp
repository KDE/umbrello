/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatastream.h>

#include "activitywidgetdata.h"

ActivityWidgetData::ActivityWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_ActivityType = ActivityWidget::Normal;
	m_Name = "Activity";
	m_Doc = "";
	m_Type = Uml::wt_Activity;
}

ActivityWidgetData::ActivityWidgetData(ActivityWidgetData & Other) : UMLWidgetData(Other) {
	*this = Other;
}

ActivityWidgetData::~ActivityWidgetData() {
}

ActivityWidgetData & ActivityWidgetData::operator=(ActivityWidgetData & Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;
	m_ActivityType = Other.m_ActivityType;
	m_Name = Other.m_Name;
	return *this;
}

bool ActivityWidgetData::operator==(ActivityWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}
	if( m_ActivityType != Other.m_ActivityType )
		return false;
	if( m_Name != Other.m_Name )
		return false;
	return true;
}

long ActivityWidgetData::getClipSizeOf() {
	long l_size = ( UMLWidgetData::getClipSizeOf() + sizeof( int ) );
	if( m_Name.length() == 0 )
		l_size += sizeof( Q_UINT32 );
	else
		l_size += ( sizeof( QChar ) * m_Name.length() );
	if( m_Doc.length() == 0 )
		l_size += sizeof( Q_UINT32 );
	else
		l_size += ( sizeof( QChar ) * m_Doc.length() );
	return l_size;
}

bool ActivityWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	int nActivity = (int)m_ActivityType;
	if( archive ) {
		*s << nActivity
		<< m_Name
		<< m_Doc;
	} else {
		*s >> nActivity
		>> m_Name
		>> m_Doc;
		m_ActivityType = (ActivityWidget::ActivityType)nActivity;
	}
	return UMLWidgetData::serialize(s, archive, fileversion);
}

bool ActivityWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement activityElement = qDoc.createElement( "UML:ActivityWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, activityElement );
	activityElement.setAttribute( "activityname", m_Name );
	activityElement.setAttribute( "documentation", m_Doc );
	activityElement.setAttribute( "activitytype", m_ActivityType );
	qElement.appendChild( activityElement );
	return status;
}

bool ActivityWidgetData::loadFromXMI( QDomElement & qElement ) {
	if( !UMLWidgetData::loadFromXMI( qElement ) )
		return false;
	m_Name = qElement.attribute( "activityname", "" );
	m_Doc = qElement.attribute( "documentation", "" );
	QString type = qElement.attribute( "activitytype", "1" );
	m_ActivityType = (ActivityWidget::ActivityType)type.toInt();
	return true;
}








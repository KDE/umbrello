/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "actorwidgetdata.h"

ActorWidgetData::ActorWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_Type = Uml::wt_Actor;
}

ActorWidgetData::~ActorWidgetData() {}

ActorWidgetData::ActorWidgetData(ActorWidgetData & Other) : UMLWidgetData(Other) {
	*this = Other;
}

ActorWidgetData & ActorWidgetData::operator=(ActorWidgetData & Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;

	return *this;
}

bool ActorWidgetData::operator==(ActorWidgetData & Other) {
	if(!(*((UMLWidgetData*)this) == (UMLWidgetData)Other)) {
		return false;
	}

	return true;
}

/** Returns the amount of bytes needed to serialize this object
If the serialization method of this class is changed this function will have to be CHANGED TOO
This function is used by the Copy and Paste Functionality
The Size in bytes of a serialized QString Object is long sz:
		if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
		{
			sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
		}
	This calculation is valid only for QT 2.1.x or superior, this is totally incompatible with QT 2.0.x or QT 1.x or inferior
	That means the copy and paste functionality will work on with QT 2.1.x or superior */
long ActorWidgetData::getClipSizeOf() {
	return UMLWidgetData::getClipSizeOf();
}

/** No descriptions */
bool ActorWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	return UMLWidgetData::serialize(s, archive, fileversion);
}

bool ActorWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement actorElement = qDoc.createElement( "UML:ActorWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, actorElement );
	qElement.appendChild( actorElement );
	return status;
}

bool ActorWidgetData::loadFromXMI( QDomElement & qElement ) {
	return UMLWidgetData::loadFromXMI( qElement );
}




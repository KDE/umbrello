/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "usecasewidgetdata.h"

UseCaseWidgetData::UseCaseWidgetData(SettingsDlg::OptionState optionState):UMLWidgetData(optionState) {
	m_Type = Uml::wt_UseCase;
}

UseCaseWidgetData::UseCaseWidgetData(UseCaseWidgetData & Other) : UMLWidgetData(Other) {
	*this = Other;
}

UseCaseWidgetData::~UseCaseWidgetData() {
}

UseCaseWidgetData & UseCaseWidgetData::operator=(UseCaseWidgetData & Other) {
	*((UMLWidgetData*)this) = (UMLWidgetData)Other;

	return *this;
}

bool UseCaseWidgetData::operator==(UseCaseWidgetData & Other) {
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
long UseCaseWidgetData::getClipSizeOf() {
	return UMLWidgetData::getClipSizeOf();
}

/** No descriptions */
bool UseCaseWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	return UMLWidgetData::serialize(s, archive, fileversion);
}

bool UseCaseWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement usecaseElement = qDoc.createElement( "UML:UseCaseWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, usecaseElement );
	qElement.appendChild( usecaseElement );
	return status;
}

bool UseCaseWidgetData::loadFromXMI( QDomElement & qElement ) {
	return UMLWidgetData::loadFromXMI( qElement );
}



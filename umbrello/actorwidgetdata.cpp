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

bool ActorWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement actorElement = qDoc.createElement( "UML:ActorWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, actorElement );
	qElement.appendChild( actorElement );
	return status;
}

bool ActorWidgetData::loadFromXMI( QDomElement & qElement ) {
	return UMLWidgetData::loadFromXMI( qElement );
}




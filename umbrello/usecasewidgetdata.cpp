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

bool UseCaseWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement usecaseElement = qDoc.createElement( "UML:UseCaseWidget" );
	bool status = UMLWidgetData::saveToXMI( qDoc, usecaseElement );
	qElement.appendChild( usecaseElement );
	return status;
}

bool UseCaseWidgetData::loadFromXMI( QDomElement & qElement ) {
	return UMLWidgetData::loadFromXMI( qElement );
}



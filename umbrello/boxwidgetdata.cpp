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

#include "boxwidgetdata.h"
BoxWidgetData::BoxWidgetData():UMLWidgetData() {
	m_Type = Uml::wt_Box;
}

BoxWidgetData::BoxWidgetData(BoxWidgetData & Other) : UMLWidgetData(Other) {
	*this = Other;
}

BoxWidgetData::~BoxWidgetData() {
}

bool BoxWidgetData::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement boxElement = qDoc.createElement("boxwidget");
	bool status = UMLWidgetData::saveToXMI(qDoc, boxElement);
	qElement.appendChild(boxElement);
	return status;
}

bool BoxWidgetData::loadFromXMI(QDomElement& qElement) {
	if( !UMLWidgetData::loadFromXMI(qElement) ) {
		return false;
	}
	return true;
}





/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "actor.h"
#include "umldoc.h"

UMLActor::UMLActor(UMLDoc * parent, const QString & name, int id) : UMLCanvasObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLActor::~UMLActor() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLActor::init() {
	m_BaseType = ot_Actor;
}

bool UMLActor::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement actorElement = qDoc.createElement("UML:Actor");
	bool status = UMLObject::saveToXMI(qDoc, actorElement);
	qElement.appendChild(actorElement);
	return status;
}

bool UMLActor::loadFromXMI(QDomElement& element) {
	return UMLObject::loadFromXMI(element);
}






/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "actor.h"

UMLActor::UMLActor(const QString & name, int id) : UMLCanvasObject(name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLActor::~UMLActor() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLActor::init() {
	m_BaseType = ot_Actor;
}

UMLObject* UMLActor::clone() const {
	UMLActor *clone = new UMLActor();
	UMLObject::copyInto(clone);
	return clone;
}

void UMLActor::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement actorElement = UMLObject::save("UML:Actor", qDoc);
	qElement.appendChild(actorElement);
}

bool UMLActor::load(QDomElement&) {
	return true;
}


#include "actor.moc"

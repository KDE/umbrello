/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "component.h"
#include "association.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLComponent::UMLComponent(QObject* parent, QString name, int id) : UMLCanvasObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLComponent::UMLComponent(QObject* parent) : UMLCanvasObject(parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLComponent::~UMLComponent() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLComponent::init() {
	m_BaseType = ot_Component;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLComponent::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement componentElement = qDoc.createElement("UML:Component");
	bool status = UMLObject::saveToXMI(qDoc, componentElement);
	qElement.appendChild(componentElement);
	return status;
}

bool UMLComponent::loadFromXMI(QDomElement& element) {
	if ( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	return true;
}

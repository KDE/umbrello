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

UMLComponent::UMLComponent(UMLDoc * parent, const QString & name, int id)
  : UMLCanvasObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLComponent::~UMLComponent() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLComponent::init() {
	m_BaseType = ot_Component;
	m_executable = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLComponent::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement componentElement = qDoc.createElement("UML:Component");
	bool status = UMLObject::saveToXMI(qDoc, componentElement);
	componentElement.setAttribute("executable", m_executable);
	qElement.appendChild(componentElement);
	return status;
}

bool UMLComponent::loadFromXMI(QDomElement& element) {
	if ( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	QString executable = element.attribute("executable", "0");
	m_executable = (bool)executable.toInt();
	return true;
}

void UMLComponent::setExecutable(bool executable) {
	m_executable = executable;
}

bool UMLComponent::getExecutable() {
	return m_executable;
}

#include "component.moc"

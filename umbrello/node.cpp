/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "node.h"
#include "association.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLNode::UMLNode(QObject* parent, QString name, int id) : UMLCanvasObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLNode::UMLNode(QObject* parent) : UMLCanvasObject(parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLNode::~UMLNode() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLNode::init() {
	m_BaseType = ot_Node;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLNode::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement nodeElement = qDoc.createElement("UML:Node");
	bool status = UMLObject::saveToXMI(qDoc, nodeElement);
	qElement.appendChild(nodeElement);
	return status;
}

bool UMLNode::loadFromXMI(QDomElement& element) {
	if ( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	return true;
}

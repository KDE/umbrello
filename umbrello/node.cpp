/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "node.h"
#include <kdebug.h>
#include <klocale.h>

UMLNode::UMLNode(const QString & name, Uml::IDType id)
        : UMLCanvasObject(name, id) {
    init();
}

UMLNode::~UMLNode() {
}

void UMLNode::init() {
    m_BaseType = Uml::ot_Node;
}

UMLObject* UMLNode::clone() const {
    UMLNode *clone = new UMLNode();
    UMLObject::copyInto(clone);
    return clone;
}

void UMLNode::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement nodeElement = UMLObject::save("UML:Node", qDoc);
    qElement.appendChild(nodeElement);
}

bool UMLNode::load(QDomElement& ) {
    return true;
}

#include "node.moc"

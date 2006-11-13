/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "actor.h"

UMLActor::UMLActor(const QString & name, Uml::IDType id) : UMLCanvasObject(name, id) {
    init();
}

UMLActor::~UMLActor() {}

void UMLActor::init() {
    m_BaseType = Uml::ot_Actor;
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

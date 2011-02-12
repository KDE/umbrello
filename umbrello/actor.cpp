/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "actor.h"

/**
 * Constructs an Actor.
 *
 * @param name              The name of the Actor.
 * @param id                The unique id to assign to this Actor.
 */
UMLActor::UMLActor(const QString & name, Uml::IDType id) : UMLCanvasObject(name, id) {
    init();
}

UMLActor::~UMLActor() {}

/**
 * Initializes key variables of the class.
 */
void UMLActor::init() {
    m_BaseType = UMLObject::ot_Actor;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLActor::clone() const {
    UMLActor *clone = new UMLActor();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Creates the <UML:Actor> XMI element.
 */
void UMLActor::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement actorElement = UMLObject::save("UML:Actor", qDoc);
    qElement.appendChild(actorElement);
}

/**
 * Loads the <UML:Actor> XMI element (empty.)
 */
bool UMLActor::load(QDomElement&) {
    return true;
}


#include "actor.moc"

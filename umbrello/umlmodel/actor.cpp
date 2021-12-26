/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "actor.h"

/**
 * Constructs an Actor.
 *
 * @param name   The name of the Actor.
 * @param id     The unique id to assign to this Actor.
 */
UMLActor::UMLActor(const QString & name, Uml::ID::Type id)
  : UMLCanvasObject(name, id)
{
    init();
}

/**
 * Standard destructor.
 */
UMLActor::~UMLActor()
{
}

/**
 * Initializes key variables of the class.
 */
void UMLActor::init()
{
    m_BaseType = UMLObject::ot_Actor;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLActor::clone() const
{
    UMLActor *clone = new UMLActor();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Creates the <UML:Actor> XMI element.
 */
void UMLActor::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QLatin1String("Actor"));
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:Actor> XMI element (empty).
 */
bool UMLActor::load1(QDomElement& element)
{
    Q_UNUSED(element);
    return true;
}


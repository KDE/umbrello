/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "port.h"

#include <KLocalizedString>

/**
 * Sets up a Port.
 *
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLPort::UMLPort(const QString & name, Uml::ID::Type id)
  : UMLCanvasObject(name, id)
{
    init();
}

/**
 * Destructor.
 */
UMLPort::~UMLPort()
{
}

/**
 * Initializes key variables of the class.
 */
void UMLPort::init()
{
    m_BaseType = UMLObject::ot_Port;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLPort::clone() const
{
    UMLPort *clone = new UMLPort();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Creates the <UML:Port> XMI element.
 */
void UMLPort::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement portElement = UMLObject::save(QLatin1String("UML:Port"), qDoc);
    qElement.appendChild(portElement);
}

/**
 * Loads the <UML:Port> XMI element (empty.)
 */
bool UMLPort::load(QDomElement&)
{
    return true;
}

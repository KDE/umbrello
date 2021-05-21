/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
void UMLPort::saveToXMI1(QXmlStreamWriter& writer)
{
    UMLObject::save1(QLatin1String("UML:Port"), writer);
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:Port> XMI element (empty.)
 */
bool UMLPort::load1(QDomElement&)
{
    return true;
}


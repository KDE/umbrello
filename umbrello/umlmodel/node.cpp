/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "node.h"

#include <KLocalizedString>

/**
 * Sets up a Node.
 *
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLNode::UMLNode(const QString & name, Uml::ID::Type id)
  : UMLCanvasObject(name, id)
{
    init();
}

/**
 * Destructor.
 */
UMLNode::~UMLNode()
{
}

/**
 * Initializes key variables of the class.
 */
void UMLNode::init()
{
    m_BaseType = UMLObject::ot_Node;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLNode::clone() const
{
    UMLNode *clone = new UMLNode();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Creates the <UML:Node> XMI element.
 */
void UMLNode::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QLatin1String("Node"));
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:Node> XMI element (empty.)
 */
bool UMLNode::load1(QDomElement&)
{
    return true;
}


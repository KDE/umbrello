/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2016                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "datatype.h"

/**
 * Sets up a Datatype.
 * @param name   The name of the Datatype.
 * @param id     The unique id of the Concept.
 */
UMLDatatype::UMLDatatype(const QString & name, Uml::ID::Type id)
  : UMLClassifier(name, id),
    m_isRef(false)
{
    m_BaseType = UMLObject::ot_Datatype;
}

/**
 * Destructor.
 */
UMLDatatype::~UMLDatatype()
{
}

/**
 * Set the origin type (in case of e.g. typedef)
 * @param origType   the origin type to set
 */
void UMLDatatype::setOriginType(UMLClassifier *origType)
{
    m_pSecondary = origType;
}

/**
 * Get the origin type (in case of e.g. typedef)
 * @return   the origin type
 */
UMLClassifier * UMLDatatype::originType() const
{
    return m_pSecondary->asUMLClassifier();
}

/**
 * Set the m_isRef flag (true when dealing with a pointer type)
 * @param isRef   the flag to set
 */
void UMLDatatype::setIsReference(bool isRef)
{
    m_isRef = isRef;
}

/**
 * Get the m_isRef flag.
 * @return   true if is reference, otherwise false
 */
bool UMLDatatype::isReference() const
{
    return m_isRef;
}

/**
 * Loads object from QDomElement.
 *
 * @param qElement A QDomElement which contains xml info for this object.
 */
bool UMLDatatype::loadFromXMI1(QDomElement & element)
{
    if (!UMLObject::loadFromXMI1(element))
        return false;

    m_SecondaryId = element.attribute(QLatin1String("elementReference"));
    if (!m_SecondaryId.isEmpty()) {
        // @todo We do not currently support composition.
        m_isRef = true;
    }
    return true;
}

/**
 * Creates the UML:Datatype element.
 *
 * @param qDoc       the xml document
 * @param qElement   the xml element
 */
void UMLDatatype::SaveToXMI1(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement classifierElement = UMLObject::save1(QLatin1String("UML:DataType"), qDoc);

    if (m_pSecondary != 0)
        classifierElement.setAttribute(QLatin1String("elementReference"),
                                        Uml::ID::toString(m_pSecondary->id()));
    qElement.appendChild(classifierElement);
}

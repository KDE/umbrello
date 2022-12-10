/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "datatype.h"

/**
 * Sets up a Datatype.
 * @param name   The name of the Datatype.
 * @param id     The unique id of the Concept.
 */
UMLDatatype::UMLDatatype(const QString & name, Uml::ID::Type id)
  : UMLClassifier(name, id),
    m_isRef(false), m_isActive(true)
{
    m_BaseType = UMLObject::ot_Datatype;
    setStereotype(QLatin1String("dataType"));
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
 * Set the m_isActive flag (is already set true by constructor).
 * @param active  the flag to set
 */
void UMLDatatype::setActive(bool active)
{
    m_isActive = active;
}

/**
 * Get the m_isActive flag.
 * @return   true if is active, otherwise false
 */
bool UMLDatatype::isActive() const
{
    return m_isActive;
}

/**
 * Loads Datatype specific attributes from QDomElement.
 * Is invoked by UMLObject::loadFromXMI() which does most of the work.
 *
 * @param element A QDomElement which contains xml info for this object.
 */
bool UMLDatatype::load1(QDomElement & element)
{
    m_SecondaryId = element.attribute(QLatin1String("elementReference"));
    if (!m_SecondaryId.isEmpty()) {
        // @todo We do not currently support composition.
        m_isRef = true;
    }
    QString active = element.attribute(QLatin1String("isActive"));
    m_isActive = (active != QLatin1String("false"));
    return true;
}

/**
 * Creates the UML:Datatype XMI element.
 * Invokes UMLObject::save1() which does most of the work.
 *
 * @param writer     the QXmlStreamWriter to save to
 */
void UMLDatatype::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QLatin1String("DataType"));

    if (m_pSecondary != 0)
        writer.writeAttribute(QLatin1String("elementReference"),
                                        Uml::ID::toString(m_pSecondary->id()));
    if (!m_isActive)
        writer.writeAttribute(QLatin1String("isActive"), QLatin1String("false"));

    UMLObject::save1end(writer);
}

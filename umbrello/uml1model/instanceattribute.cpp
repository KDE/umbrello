/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

//own header
#include "instanceattribute.h"

//local includes
#include "debug_utils.h"
#include "umldoc.h"
#include "uml.h"
#include "umlinstanceattributedialog.h"
#include "object_factory.h"

//kde includes
#include <KLocalizedString>
#include <KMessageBox>

UMLInstanceAttribute::UMLInstanceAttribute(UMLObject *parent, const QString &name, Uml::ID::Type id, Uml::Visibility::Enum s, UMLObject *type, const QString &value)
    : UMLAttribute(parent, name, id, s, type, value)
{
    init();
    if(m_pSecondary){
        m_pSecondary->setBaseType(UMLObject::ot_Instance);
    }
}

UMLInstanceAttribute::UMLInstanceAttribute(UMLObject *parent)
    : UMLAttribute(parent)
{
    init();
}

/**
 * @brief UMLInstanceAttribute::init
 * Initialize members of this class
 */
void UMLInstanceAttribute::init()
{
    m_BaseType = UMLObject::ot_InstanceAttribute;
}

/**
 * Sets the UMLInstanceAttribute's attributes property.
 * @param attributes  The new value for the attributes property.
 */
void UMLInstanceAttribute::setAttributes(const QString &attributes)
{
    m_attributes = attributes;
}

/**
 * Returns the value of the UMLInstanceAttribute's attributes property.
 * @return  The value of the UMLInstanceAttribute's attributes property.
 */
QString UMLInstanceAttribute::getAttributes() const
{
    return m_attributes;
}

/**
 * Creates the <UML:InstanceAttribute> XMI element.
 */
void UMLInstanceAttribute::saveToXMI1(QXmlStreamWriter& writer)
{
    UMLObject::save1(QLatin1String("UML:InstanceAttribute"), writer);
    if (m_pSecondary == NULL) {
        uDebug() << name() << ": m_pSecondary is NULL, using local name " << m_SecondaryId;
        writer.writeAttribute(QLatin1String("type"), m_SecondaryId);
    } else {
        writer.writeAttribute(QLatin1String("type"), Uml::ID::toString(m_pSecondary->id()));
    }
    writer.writeAttribute(QLatin1String("initialValue"), m_InitialValue);
    writer.writeAttribute(QLatin1String("attributes"), m_attributes);
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:InstanceAttribute> XMI element.
 */
bool UMLInstanceAttribute::load1(QDomElement & element)
{
    if (! UMLAttribute::load1(element))
        return false;
    m_attributes = element.attribute(QLatin1String("attributes"));
    m_InitialValue = element.attribute(QLatin1String("initialValue"));
    return true;
}

/**
 * Display the properties configuration dialog for the instanceattribute.
 */
bool UMLInstanceAttribute::showPropertiesDialog(QWidget* parent)
{
    UMLInstanceAttributeDialog dialog(parent, this);
    return dialog.exec();
}


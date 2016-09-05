/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "enumliteral.h"
#include "umlenumliteraldialog.h"

// kde includes
#if QT_VERSION < 0x050000
#include <kinputdialog.h>
#endif
#include <KLocalizedString>

// qt includes
#if QT_VERSION >= 0x050000
#include <QInputDialog>
#endif

/**
 * Sets up an enum literal.
 * @param parent    The parent of this UMLEnumLiteral.
 * @param name      The name of this UMLEnumLiteral.
 * @param id        The unique id given to this UMLEnumLiteral.
 * @param v         The value fo this UMLEnumLiteral.
 */
UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent,
                               const QString& name, Uml::ID::Type id,
                               const QString& v)
  : UMLClassifierListItem(parent, name, id)
{
    m_Value = v;
    m_BaseType = UMLObject::ot_EnumLiteral;
}

/**
 * Sets up an enum literal.
 * @param parent    The parent of this UMLEnumLiteral.
 */
UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent)
  : UMLClassifierListItem(parent)
{
    m_BaseType = UMLObject::ot_EnumLiteral;
}

/**
 * Destructor.
 */
UMLEnumLiteral::~UMLEnumLiteral()
{
}

/**
 * Returns the value of the UMLEnumLiteral.
 *
 * @return  The value of the Enum Literal.
 */
QString UMLEnumLiteral::value() const
{
    return m_Value;
}

/**
 * Sets the value of the UMLEnumLiteral.
 *
 * @param v   The value of the Enum Literal.
 */
void UMLEnumLiteral::setValue(const QString &v)
{
    if(m_Value != v) {
        m_Value = v;
        UMLObject::emitModified();
    }
}

/**
 * Returns a string representation of the UMLEnumLiteral.
 *
 * @param sig   If true will show the attribute type and value.
 * @return  Returns a string representation of the UMLEnumLiteral.
 */
QString UMLEnumLiteral::toString(Uml::SignatureType::Enum sig)
{
    QString s;
    Q_UNUSED(sig);

    s = name();
    if (m_Value.length() > 0)
        s += QLatin1String(" = ") + m_Value;
    return s;
}

/**
 * Overloaded '==' operator
 */
bool UMLEnumLiteral::operator==(const UMLEnumLiteral& rhs) const
{
    if (this == &rhs)  {
        return true;
    }
    if (!UMLObject::operator==(rhs))  {
        return false;
    }
    return true;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLEnumLiteral::copyInto(UMLObject *lhs) const
{
    UMLEnumLiteral *target = lhs->asUMLEnumLiteral();
    UMLClassifierListItem::copyInto(lhs);

    target->m_Value = m_Value;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLEnumLiteral::clone() const
{
    UMLEnumLiteral *clone = new UMLEnumLiteral((UMLObject *) parent());
    copyInto(clone);

    return clone;
}

/**
 * Creates the <UML:EnumLiteral> XMI element.
 */
void UMLEnumLiteral::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement attributeElement = UMLObject::save(QLatin1String("UML:EnumerationLiteral"), qDoc);
    if (! m_Value.isEmpty())
        attributeElement.setAttribute(QLatin1String("value"), m_Value);
    qElement.appendChild(attributeElement);
}

/**
 * Loads the <UML:EnumLiteral> XMI element (empty.)
 */
bool UMLEnumLiteral::load(QDomElement& element)
{
    m_Value = element.attribute(QLatin1String("value"));
    return true;
}

/**
 * Display the properties configuration dialog for the enum literal.
 */
bool UMLEnumLiteral::showPropertiesDialog(QWidget* parent)
{
    UMLEnumLiteralDialog dialog(parent, this);
    return dialog.exec();
}

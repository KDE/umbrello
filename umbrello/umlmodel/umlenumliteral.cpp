/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlenumliteral.h"
#include "umlenumliteraldialog.h"

#include <KLocalizedString>


/**
 * Sets up an enum literal.
 * @param parent    The parent of this UMLEnumLiteral.
 * @param name      The name of this UMLEnumLiteral.
 * @param id        The unique id given to this UMLEnumLiteral.
 * @param v         The numeric representation value of this UMLEnumLiteral.
 *                  CAVEAT: 1) This does not conform to the UML standard.
 *                          2) Programming language specific:
 *                             Not all languages support enum representations.
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
QString UMLEnumLiteral::toString(Uml::SignatureType::Enum sig,
                                 bool /*withStereotype*/) const
{
    QString s;
    Q_UNUSED(sig);

    s = name();
    if (m_Value.length() > 0)
        s += QStringLiteral(" = ") + m_Value;
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
    UMLEnumLiteral *clone = new UMLEnumLiteral(umlParent());
    copyInto(clone);

    return clone;
}

/**
 * Creates the <UML:EnumLiteral> XMI element.
 */
void UMLEnumLiteral::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("EnumerationLiteral"), QStringLiteral("ownedLiteral"));
    if (! m_Value.isEmpty())
        writer.writeAttribute(QStringLiteral("value"), m_Value);
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:EnumLiteral> XMI element (empty.)
 */
bool UMLEnumLiteral::load1(QDomElement& element)
{
    m_Value = element.attribute(QStringLiteral("value"));
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

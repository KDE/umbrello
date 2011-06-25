/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "enumliteral.h"

#include <kinputdialog.h>
#include <klocale.h>

/**
 * Sets up an enum literal.
 * @param parent    The parent of this UMLEnumLiteral.
 * @param name      The name of this UMLEnumLiteral.
 * @param id        The unique id given to this UMLEnumLiteral.
 */
UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent,
                               const QString& name, Uml::IDType id)
  : UMLClassifierListItem(parent, name, id)
{
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
 * Overloaded '==' operator
 */
bool UMLEnumLiteral::operator==(const UMLEnumLiteral& rhs) const
{
    if ( this == &rhs )  {
        return true;
    }
    if ( !UMLObject::operator==( rhs ) )  {
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
    UMLClassifierListItem::copyInto(lhs);
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
    QDomElement attributeElement = UMLObject::save("UML:EnumerationLiteral", qDoc);
    qElement.appendChild( attributeElement );
}

/**
 * Loads the <UML:EnumLiteral> XMI element (empty.)
 */
bool UMLEnumLiteral::load(QDomElement& element)
{
    Q_UNUSED(element);
    return true;
}

/**
 * Display the properties configuration dialog for the enum literal.
 */
bool UMLEnumLiteral::showPropertiesDialog(QWidget* parent)
{
    bool ok;
    QString enumName = KInputDialog::getText(i18nc("enum name", "Name"), i18n("Enter name:"), name(), &ok, parent);
    if ( ok && !enumName.isEmpty() )  {
        setName(enumName);
        return true;
    } else {
        return false;
    }
}

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "enumliteral.h"

#include <kinputdialog.h>
#include <kdebug.h>
#include <klocale.h>

UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent,
                               const QString& name, Uml::IDType id)
        : UMLClassifierListItem(parent, name, id)
{
    m_BaseType = Uml::ot_EnumLiteral;
}

UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent) : UMLClassifierListItem(parent)
{
    m_BaseType = Uml::ot_EnumLiteral;
}

UMLEnumLiteral::~UMLEnumLiteral()
{
}

bool UMLEnumLiteral::operator==(const UMLEnumLiteral& rhs)
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
bool UMLEnumLiteral::load(QDomElement& )
{
    return true;
}

/**
 * Display the properties configuration dialog for the enum literal.
 */
bool UMLEnumLiteral::showPropertiesDialog(QWidget* parent)
{
    bool ok;
    QString name = KInputDialog::getText(i18nc("enum name", "Name"), i18n("Enter name:"), getName(), &ok, parent);
    if ( ok && !name.isEmpty() )  {
        setName(name);
        return true;
    } else {
        return false;
    }
}


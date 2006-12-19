/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "enumliteral.h"

#include <kinputdialog.h>
#include <kdebug.h>
#include <klocale.h>

UMLEnumLiteral::UMLEnumLiteral(const UMLObject *parent,
                               const QString& name, Uml::IDType id)
        : UMLClassifierListItem(parent, name, id) {
    m_BaseType = Uml::ot_EnumLiteral;
}

UMLEnumLiteral::UMLEnumLiteral(const UMLObject *parent) : UMLClassifierListItem(parent) {
    m_BaseType = Uml::ot_EnumLiteral;
}

UMLEnumLiteral::~UMLEnumLiteral() { }

bool UMLEnumLiteral::operator==(UMLEnumLiteral& rhs) {
    if ( this == &rhs )  {
        return true;
    }
    if ( !UMLObject::operator==( rhs ) )  {
        return false;
    }
    return true;
}

void UMLEnumLiteral::copyInto(UMLEnumLiteral *rhs) const
{
    UMLClassifierListItem::copyInto(rhs);
}

UMLObject* UMLEnumLiteral::clone() const
{
    UMLEnumLiteral *clone = new UMLEnumLiteral((UMLObject *) parent());
    copyInto(clone);

    return clone;
}


void UMLEnumLiteral::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement attributeElement = UMLObject::save("UML:EnumerationLiteral", qDoc);
    qElement.appendChild( attributeElement );
}

bool UMLEnumLiteral::load(QDomElement& ) {
    return true;
}

bool UMLEnumLiteral::showPropertiesDialog(QWidget* parent) {
    bool ok;
    QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), getName(), &ok, parent);
    if ( ok && !name.isEmpty() )  {
        setName(name);
        return true;
    } else {
        return false;
    }
}



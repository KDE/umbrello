/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef ENUMLITERAL_H
#define ENUMLITERAL_H

#include "classifierlistitem.h"

/**
 * This class is used to set up information for an enum literal.  Enum
 * literals are the values that enums can be set to.
 *
 * @short Sets up attribute information.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLObject
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class UMLEnumLiteral : public UMLClassifierListItem
{
public:
    UMLEnumLiteral(UMLObject* parent,
                   const QString& name, Uml::ID::Type id = Uml::ID::None,
                   const QString& v = QString());
    explicit UMLEnumLiteral(UMLObject* parent);

    bool operator==(const UMLEnumLiteral &rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    virtual ~UMLEnumLiteral();

    QString value() const;
    void setValue(const QString &v);

    QString toString(Uml::SignatureType::Enum sig = Uml::SignatureType::NoSig, bool withStereotype=false);

    virtual void saveToXMI1(QDomDocument& qDoc, QDomElement& qElement);

    virtual bool showPropertiesDialog(QWidget* parent = 0);

protected:
    bool load1(QDomElement& element);

    /**
     *   Value is an extension to the UML 2.5 metamodel.
     *   It manage the internal representation of enumeration value
     *   in system programming languages.
     */

    QString m_Value;

};

#endif

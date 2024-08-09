/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
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

    QString toString(Uml::SignatureType::Enum sig = Uml::SignatureType::NoSig,
                     bool withStereotype=false) const;

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual bool showPropertiesDialog(QWidget *parent = nullptr);

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

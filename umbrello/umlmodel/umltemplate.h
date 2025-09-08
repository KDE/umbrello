/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "umlclassifierlistitem.h"

/**
 * This class holds information used by template classes, called
 * paramaterised class in UML and a generic in Java.  It has a
 * type (usually just "class") and name.
 *
 * @short Sets up template information.
 * @author Jonathan Riddell
 * @see UMLObject
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class UMLTemplate : public UMLClassifierListItem
{
public:

    UMLTemplate(UMLObject *parent, const QString& name,
                Uml::ID::Type id = Uml::ID::None, const QString& type = QStringLiteral("class"));

    explicit UMLTemplate(UMLObject *parent);

    bool operator==(const UMLTemplate &rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    virtual ~UMLTemplate();

    QString toString(Uml::SignatureType::Enum sig = Uml::SignatureType::NoSig,
                     bool withStereotype=false) const;

    virtual QString getTypeName() const;

    virtual bool showPropertiesDialog(QWidget* parent);

    void saveToXMI(QXmlStreamWriter& writer);

protected:

    bool load1(QDomElement & element);

};

#endif

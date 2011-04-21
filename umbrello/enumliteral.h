/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEnumLiteral : public UMLClassifierListItem
{
public:
    UMLEnumLiteral(UMLObject* parent,
                   const QString& name, Uml::IDType id = Uml::id_None);
    UMLEnumLiteral(UMLObject* parent);

    bool operator==(const UMLEnumLiteral &rhs) const;

    virtual void copyInto(UMLObject *lhs) const;

    virtual UMLObject* clone() const;

    virtual ~UMLEnumLiteral();

    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

    bool showPropertiesDialog(QWidget* parent);

protected:
    bool load(QDomElement& element);

};

#endif

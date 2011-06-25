/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ENTITYCONSTRAINT_H
#define ENTITYCONSTRAINT_H

#include "basictypes.h"
#include "classifierlistitem.h"
#include "umlclassifierlist.h"

/**
 * This class is used to set up information for a entity constraint.
 *
 * @short Sets up entity constraint information.
 * @author Sharan Rao
 * @see UMLObject UMLClassifierListItem
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEntityConstraint : public UMLClassifierListItem
{
     Q_OBJECT

public:
    UMLEntityConstraint(UMLObject *parent, const QString& name,
                        Uml::IDType id = Uml::id_None );

    UMLEntityConstraint(UMLObject *parent);

    bool operator==( const UMLEntityConstraint &rhs) const;

    virtual ~UMLEntityConstraint();

    virtual void copyInto(UMLObject *lhs) const;

    /**
     * Make a clone of the UMLEntityConstraint.
     */
    virtual UMLObject* clone() const = 0;

};

#endif

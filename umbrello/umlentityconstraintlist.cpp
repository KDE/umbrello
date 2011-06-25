/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlentityconstraintlist.h"

#include "entityconstraint.h"

#include <klocale.h>

UMLEntityConstraintList::UMLEntityConstraintList()
{
}

UMLEntityConstraintList::UMLEntityConstraintList(const UMLEntityConstraintList& other)
    : QList<UMLEntityConstraint*>( other )
{
}

UMLEntityConstraintList::~UMLEntityConstraintList()
{
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLEntityConstraintList::copyInto(UMLEntityConstraintList* rhs) const
{
    // Don't copy yourself.
    if (rhs == this) return;

    rhs->clear();

    // Suffering from const; we shall not modify our object.
    UMLEntityConstraintList* tmp = new UMLEntityConstraintList(*this);

    UMLEntityConstraint* item;
    for (UMLEntityConstraintListIt ecit( *tmp ); ecit.hasNext() ; ) {
        item = ecit.next();
        rhs->append((UMLEntityConstraint*)item->clone());
    }
    delete tmp;
}

/**
 * Make a clone of this object.
 */
UMLEntityConstraintList* UMLEntityConstraintList::clone() const
{
    UMLEntityConstraintList *clone = new UMLEntityConstraintList();
    copyInto(clone);
    return clone;
}

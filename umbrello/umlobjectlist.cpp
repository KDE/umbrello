/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlobjectlist.h"
#include "umlobject.h"

UMLObjectList::UMLObjectList()
{
}

UMLObjectList::~UMLObjectList()
{
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLObjectList::copyInto(UMLObjectList *rhs) const
{
    // Don't copy yourself.
    if (rhs == this) return;

    rhs->clear();

    // Suffering from const; we shall not modify our object.
    UMLObjectList *tmp = new UMLObjectList(*this);

    UMLObject *item = NULL;
    for (UMLObjectListIt oit( *tmp ); oit.hasNext() ; )
    {
        item = oit.next();
        rhs->append(item->clone());
    }
    delete tmp;
}

/**
 * Make a clone of this object.
 */
UMLObjectList* UMLObjectList::clone() const
{
    UMLObjectList *clone = new UMLObjectList();
    copyInto(clone);
    return clone;
}

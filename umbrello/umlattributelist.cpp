/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlattributelist.h"

#include "attribute.h"

#include <klocale.h>

UMLAttributeList::UMLAttributeList()
{
}

UMLAttributeList::UMLAttributeList(const UMLAttributeList& other)
    : QList<UMLAttribute*>( other )
{
}

UMLAttributeList::~UMLAttributeList()
{
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLAttributeList::copyInto(UMLAttributeList *rhs) const
{
    // Don't copy yourself.
    if (rhs == this) return;

    rhs->clear();

    // Suffering from const; we shall not modify our object.
    UMLAttributeList *tmp = new UMLAttributeList(*this);

    UMLAttribute *item;
    for (UMLAttributeListIt ait( *tmp ); ait.hasNext() ; )
    {
        item = ait.next();
        rhs->append((UMLAttribute*)item->clone());
    }
    delete tmp;
}

/**
 * Make a clone of this object.
 */
UMLAttributeList* UMLAttributeList::clone() const
{
    UMLAttributeList *clone = new UMLAttributeList();
    copyInto(clone);
    return clone;
}

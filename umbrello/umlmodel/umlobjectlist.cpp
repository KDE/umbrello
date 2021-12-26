/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    UMLObject *item = 0;
    for (UMLObjectListIt oit(*tmp); oit.hasNext() ;)
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

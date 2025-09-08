/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlattributelist.h"

#include "umlattribute.h"

#include <KLocalizedString>

UMLAttributeList::UMLAttributeList()
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
    for (UMLAttributeListIt ait(*tmp); ait.hasNext() ;)
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

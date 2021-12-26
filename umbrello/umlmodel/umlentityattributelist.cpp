/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlentityattributelist.h"

#include "entityattribute.h"

#include <KLocalizedString>

UMLEntityAttributeList::UMLEntityAttributeList()
{
}

UMLEntityAttributeList::UMLEntityAttributeList(const UMLEntityAttributeList& other)
  : QList<UMLEntityAttribute*>(other)
{
}

UMLEntityAttributeList::~UMLEntityAttributeList()
{
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLEntityAttributeList::copyInto(UMLEntityAttributeList* rhs) const
{
    // Don't copy yourself.
    if (rhs == this) return;

    rhs->clear();

    // Suffering from const; we shall not modify our object.
    UMLEntityAttributeList* tmp = new UMLEntityAttributeList(*this);

    UMLEntityAttribute* item;
    for (UMLEntityAttributeListIt eait(*tmp); eait.hasNext() ;) {
        item = eait.next();
        rhs->append((UMLEntityAttribute*)item->clone());
    }
    delete tmp;
}

/**
 * Make a clone of this object.
 */
UMLEntityAttributeList* UMLEntityAttributeList::clone() const
{
    UMLEntityAttributeList *clone = new UMLEntityAttributeList();
    copyInto(clone);
    return clone;
}

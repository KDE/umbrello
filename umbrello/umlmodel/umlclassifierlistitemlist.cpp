/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlclassifierlistitemlist.h"

#include "umlclassifierlistitem.h"

#include <KLocalizedString>

UMLClassifierListItemList::UMLClassifierListItemList()
{
}

UMLClassifierListItemList::~UMLClassifierListItemList()
{
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLClassifierListItemList::copyInto(UMLClassifierListItemList *rhs) const
{
    // Prevent copying to yourself. (Can cause serious injuries)
    if (rhs == this) return;

    rhs->clear();

    // Suffering from const; we shall not modify our object.
    UMLClassifierListItemList *tmp = new UMLClassifierListItemList(*this);

    UMLClassifierListItem *item;
    for (UMLClassifierListItemListIt clit(*tmp); clit.hasNext() ;) {
        item = clit.next();
        rhs->append((UMLClassifierListItem*)item->clone());
    }
    delete tmp;
}

/**
 * Make a clone of this object.
 */
UMLClassifierListItemList* UMLClassifierListItemList::clone() const
{
    UMLClassifierListItemList *clone = new UMLClassifierListItemList();
    copyInto(clone);
    return clone;
}

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlclassifierlistitemlist.h"

#include "classifierlistitem.h"

#include <klocale.h>

UMLClassifierListItemList::UMLClassifierListItemList()
{
}

UMLClassifierListItemList::UMLClassifierListItemList(const UMLClassifierListItemList& other)
  : QList<UMLClassifierListItem*>( other )
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
    for (UMLClassifierListItemListIt clit( *tmp ); clit.hasNext() ; ) {
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

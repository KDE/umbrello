/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlentityattributelist.h"
#include "entityattribute.h"
#include <kdebug.h>
#include <klocale.h>

void UMLEntityAttributeList::copyInto(UMLEntityAttributeList* rhs) const {
    // Don't copy yourself.
    if (rhs == this) return;

    rhs->clear();

    // Suffering from const; we shall not modify our object.
    UMLEntityAttributeList* tmp = new UMLEntityAttributeList(*this);

    UMLEntityAttribute* item;
    for (item = tmp->first(); item; item = tmp->next() ) {
        rhs->append((UMLEntityAttribute*)item->clone());
    }
    delete tmp;
}


UMLEntityAttributeList* UMLEntityAttributeList::clone() const {
    UMLEntityAttributeList *clone = new UMLEntityAttributeList();
    copyInto(clone);
    return clone;
}

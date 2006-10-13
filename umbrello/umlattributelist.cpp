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

#include "umlattributelist.h"
#include "attribute.h"
#include <kdebug.h>
#include <klocale.h>

void UMLAttributeList::copyInto(UMLAttributeList *rhs) const {
    // Don't copy yourself.
    if (rhs == this) return;

    rhs->clear();

    // Suffering from const; we shall not modify our object.
    UMLAttributeList *tmp = new UMLAttributeList(*this);

    UMLAttribute *item;
    for (item = tmp->first(); item; item = tmp->next() )
    {
        rhs->append((UMLAttribute*)item->clone());
    }
    delete tmp;
}


UMLAttributeList* UMLAttributeList::clone() const {
    UMLAttributeList *clone = new UMLAttributeList();
    copyInto(clone);
    return clone;
}

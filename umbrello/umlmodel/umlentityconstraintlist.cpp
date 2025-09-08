/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlentityconstraintlist.h"

#include "umlentityconstraint.h"

#include <KLocalizedString>

UMLEntityConstraintList::UMLEntityConstraintList()
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
    for (UMLEntityConstraintListIt ecit(*tmp); ecit.hasNext() ;) {
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

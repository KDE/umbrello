/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLENTITYCONSTRAINTLIST_H
#define UMLENTITYCONSTRAINTLIST_H

#include <qlist.h>

// forward declaration
class UMLEntityConstraint;


typedef QListIterator<UMLEntityConstraint*> UMLEntityConstraintListIt;

/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLEntityConstraint>
 * base class.
 */
class UMLEntityConstraintList : public QList<UMLEntityConstraint*>
{
public:

    UMLEntityConstraintList();

    virtual ~UMLEntityConstraintList();

    virtual void copyInto (UMLEntityConstraintList* rhs) const;

    virtual UMLEntityConstraintList* clone() const;
};


#endif

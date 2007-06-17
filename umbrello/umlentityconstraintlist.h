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

#ifndef UMLENTITYCONSTRAINTLIST_H
#define UMLENTITYCONSTRAINTLIST_H

#include <q3ptrlist.h>

// forward declaration
class UMLEntityConstraint;


typedef Q3PtrListIterator<UMLEntityConstraint> UMLEntityConstraintListIt;

/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLEntityConstraint>
 * base class.
 */
class UMLEntityConstraintList : public Q3PtrList<UMLEntityConstraint>
{
public:

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto (UMLEntityConstraintList* rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLEntityConstraintList* clone() const;
};


#endif

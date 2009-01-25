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

    UMLEntityConstraintList(const UMLEntityConstraintList& );

    virtual ~UMLEntityConstraintList();

    virtual void copyInto (UMLEntityConstraintList* rhs) const;

    virtual UMLEntityConstraintList* clone() const;
};


#endif

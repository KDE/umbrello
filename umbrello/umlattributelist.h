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

#ifndef UMLATTRIBUTELIST_H
#define UMLATTRIBUTELIST_H

#include <qptrlist.h>

#include "attribute.h"

//typedef QPtrList<UMLAttribute> UMLAttributeList;
typedef QPtrListIterator<UMLAttribute> UMLAttributeListIt;

/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLAttribute>
 * base class.
 */
class UMLAttributeList : public QPtrList<UMLAttribute>
{
public:

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto (UMLAttributeList *rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLAttributeList* clone() const;
};


#endif

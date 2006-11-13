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

#ifndef UMLENTITYATTRIBUTELIST_H
#define UMLENTITYATTRIBUTELIST_H

#include <qptrlist.h>

// forward declaration
class UMLEntityAttribute;

//typedef QPtrList<UMLEntityAttribute> UMLEntityAttributeList;
typedef QPtrListIterator<UMLEntityAttribute> UMLEntityAttributeListIt;

/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLEntityAttribute>
 * base class.
 */
class UMLEntityAttributeList : public QPtrList<UMLEntityAttribute>
{
public:

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto (UMLEntityAttributeList* rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLEntityAttributeList* clone() const;
};


#endif

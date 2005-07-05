/***************************************************************************
                          umlobjectlist.h  -  description
                             -------------------
    begin                : Sat Dec 29 2001
    copyright            : (C) 2001 by Gustavo Madrigal
    email                : gmadrigal@nextphere.com
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLOBJECTLIST_H
#define UMLOBJECTLIST_H

#include <qptrlist.h>

// forward declarations
class UMLObject;

//typedef QPtrList<UMLObject> UMLObjectList;
typedef QPtrListIterator<UMLObject> UMLObjectListIt;


/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLObject>
 * base class.
 */
class UMLObjectList : public QPtrList<UMLObject>
{

public:

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto (UMLObjectList *rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLObjectList* clone() const;
};


#endif

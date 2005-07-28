/*
 *  copyright (C) 2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLCLASSIFIERLISTITEMLIST_H
#define UMLCLASSIFIERLISTITEMLIST_H

#include <q3ptrlist.h>

// forward declaration
class UMLClassifierListItem;

//typedef QPtrList<UMLClassifierListItem> UMLClassifierListItemList;
typedef Q3PtrListIterator<UMLClassifierListItem> UMLClassifierListItemListIt;

/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLClassifierListItem>
 * base class.
 */
class UMLClassifierListItemList : public Q3PtrList<UMLClassifierListItem>
{

public:

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto (UMLClassifierListItemList *rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLClassifierListItemList* clone() const;

};

#endif

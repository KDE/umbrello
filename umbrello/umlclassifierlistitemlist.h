/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLCLASSIFIERLISTITEMLIST_H
#define UMLCLASSIFIERLISTITEMLIST_H

#include <QtCore/QList>

// forward declaration
class UMLClassifierListItem;

//typedef QPtrList<UMLClassifierListItem> UMLClassifierListItemList;
typedef QListIterator<UMLClassifierListItem*> UMLClassifierListItemListIt;

/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLClassifierListItem>
 * base class.
 */
class UMLClassifierListItemList : public QList<UMLClassifierListItem*>
{
public:

    UMLClassifierListItemList();
    UMLClassifierListItemList(const UMLClassifierListItemList& other);

    virtual ~UMLClassifierListItemList();

    virtual void copyInto (UMLClassifierListItemList *rhs) const;

    virtual UMLClassifierListItemList* clone() const;

};

#endif

/*
    SPDX-License-Identifier: GPL-2.0-or-later

    copyright (C) 2004-2014
    Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLCLASSIFIERLISTITEMLIST_H
#define UMLCLASSIFIERLISTITEMLIST_H

#include <QList>

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

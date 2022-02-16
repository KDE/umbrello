/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2001 Gustavo Madrigal gmadrigal @nextphere.com
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLOBJECTLIST_H
#define UMLOBJECTLIST_H

#include <QList>
#include <QPointer>

// forward declarations
class UMLObject;

typedef QListIterator<QPointer<UMLObject> >
UMLObjectListIt;

/**
 * This sub-class adds copyInto and clone to the QList<UMLObject*>
 * base class.
 */
class UMLObjectList : public QList<QPointer<UMLObject> >
{
public:

    UMLObjectList();
    virtual ~UMLObjectList();

    virtual void copyInto(UMLObjectList *rhs) const;

    virtual UMLObjectList* clone() const;
};

#endif

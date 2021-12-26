/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLATTRIBUTELIST_H
#define UMLATTRIBUTELIST_H

#include <QList>

#include "attribute.h"

//typedef QPtrList<UMLAttribute> UMLAttributeList;
typedef QListIterator<UMLAttribute*> UMLAttributeListIt;

/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLAttribute>
 * base class.
 */
class UMLAttributeList : public QList<UMLAttribute*>
{
public:

    UMLAttributeList();

    virtual ~UMLAttributeList();

    virtual void copyInto(UMLAttributeList *rhs) const;

    virtual UMLAttributeList* clone() const;
};


#endif

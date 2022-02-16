/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLENTITYATTRIBUTELIST_H
#define UMLENTITYATTRIBUTELIST_H

#include <qlist.h>
#include "entityattribute.h"

//typedef QPtrList<UMLEntityAttribute> UMLEntityAttributeList;
typedef QListIterator<UMLEntityAttribute*> UMLEntityAttributeListIt;

/**
 * This sub-class adds copyInto and clone to the QPtrList<UMLEntityAttribute>
 * base class.
 */
class UMLEntityAttributeList : public QList<UMLEntityAttribute*>
{
public:

    UMLEntityAttributeList();

    UMLEntityAttributeList(const UMLEntityAttributeList&);

    virtual ~UMLEntityAttributeList();

    virtual void copyInto (UMLEntityAttributeList* rhs) const;

    virtual UMLEntityAttributeList* clone() const;
};


#endif

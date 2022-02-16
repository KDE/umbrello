/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2014-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlfinder.h"

#include "umlobject.h"

UMLFinder::UMLFinder() : m_index(-1)
{
}

UMLFinder::~UMLFinder()
{
}

bool UMLFinder::includeObject(UMLFinder::Category category, UMLObject *o)
{
    if (!o)
        return false;
    UMLObject::ObjectType type = o->baseType();
    return category == All
            || (category == Classes && type == UMLObject::ot_Class)
            || (category == Interfaces && type == UMLObject::ot_Interface)
            || (category == Packages && type == UMLObject::ot_Package)
            || (category == Operations && type == UMLObject::ot_Operation)
            || (category == Attributes && type == UMLObject::ot_Attribute)
            ;
}

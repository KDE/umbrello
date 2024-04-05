/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

//own header
#include "entityconstraint.h"

// app includes
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"

// qt includes
#include <QRegularExpression>

/**
 * Sets up a constraint.
 * @param parent    The parent of this UMLEntityConstraint.
 * @param name      The name of this UMLEntityConstraint.
 * @param id        The unique id given to this UMLEntityConstraint.
 */
UMLEntityConstraint::UMLEntityConstraint(UMLObject *parent,
    const QString& name, Uml::ID::Type id)
  : UMLClassifierListItem(parent, name, id)
{
    m_BaseType = UMLObject::ot_EntityConstraint;
}

/**
 * Sets up a constraint.
 * @param parent    The parent of this UMLEntityConstraint.
 */
UMLEntityConstraint::UMLEntityConstraint(UMLObject *parent)
  : UMLClassifierListItem(parent)
{
    m_BaseType = UMLObject::ot_EntityConstraint;
}

/**
 * Overloaded '==' operator
 */
bool UMLEntityConstraint::operator==(const UMLEntityConstraint &rhs) const
{
    if(this == &rhs)
        return true;

    if(!UMLObject::operator==(rhs))
        return false;

    return true;
}

/**
 * destructor.
 */
UMLEntityConstraint::~UMLEntityConstraint()
{
}

/**
 * Copy the internal presentation of this object into the UMLEntityConstraint
 * object.
 */
void UMLEntityConstraint::copyInto(UMLObject *lhs) const
{
    // call the parent first.
    UMLClassifierListItem::copyInto(lhs);
}



/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

//own header
#include "entityconstraint.h"

// app includes
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"

// qt includes
#include <QtCore/QRegExp>

/**
 * Sets up a constraint.
 * @param parent    The parent of this UMLEntityConstraint.
 * @param name      The name of this UMLEntityConstraint.
 * @param id        The unique id given to this UMLEntityConstraint.
 */
UMLEntityConstraint::UMLEntityConstraint(UMLObject *parent,
    const QString& name, Uml::IDType id )
  : UMLClassifierListItem( parent, name, id )
{
    m_BaseType = UMLObject::ot_EntityConstraint;
}

/**
 * Sets up a constraint.
 * @param parent    The parent of this UMLEntityConstraint.
 */
UMLEntityConstraint::UMLEntityConstraint(UMLObject *parent)
  : UMLClassifierListItem( parent )
{
    m_BaseType = UMLObject::ot_EntityConstraint;
}

/**
 * Overloaded '==' operator
 */
bool UMLEntityConstraint::operator==( const UMLEntityConstraint &rhs) const
{
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
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

#include "entityconstraint.moc"


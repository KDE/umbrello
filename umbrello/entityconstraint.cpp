/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

//own header
#include "entityconstraint.h"
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"


UMLEntityConstraint::UMLEntityConstraint(UMLObject *parent,
    const QString& name, Uml::IDType id )
    : UMLClassifierListItem( parent, name, id ) {

    m_BaseType = Uml::ot_EntityConstraint;
}

UMLEntityConstraint::UMLEntityConstraint(UMLObject *parent)
    : UMLClassifierListItem( parent ) {

    m_BaseType = Uml::ot_EntityConstraint;

}

bool UMLEntityConstraint::operator==( const UMLEntityConstraint &rhs) {
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    return true;

}

UMLEntityConstraint::~UMLEntityConstraint() {}


void UMLEntityConstraint::copyInto(UMLEntityConstraint *rhs) const {

    // call the parent first.
    UMLClassifierListItem::copyInto(rhs);

}

#include "entityconstraint.moc"


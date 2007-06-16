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
#include "dialogs/umlattributedialog.h"
#include "object_factory.h"


UMLEntityConstraint::UMLEntityConstraint(const UMLObject *parent,
    const QString& name, Uml::IDType id )
    : UMLClassifierListItem( parent, name, id ) {

    m_BaseType = Uml::ot_EntityConstraint;
}

UMLEntityConstraint::UMLEntityConstraint(const UMLObject *parent)
    : UMLClassifierListItem( parent ) {

    m_BaseType = Uml::ot_EntityConstraint;

}

bool UMLEntityConstraint::operator==( UMLEntityConstraint &rhs) {
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    return true;

}

UMLEntityConstraint::~UMLEntityConstraint() {}

UMLObject* UMLEntityConstraint::clone() const {
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLEntityConstraint *clone = new UMLEntityConstraint( static_cast<UMLObject*>(parent()) );
    copyInto(clone);
    return clone;
}

QString UMLEntityConstraint::getFullyQualifiedName(QString separator,
                                                   bool includeRoot ) const {
    kDebug() << k_funcinfo << "Not Yet implemented "<< endl;

}

void UMLEntityConstraint::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
      kDebug() << k_funcinfo << "Not Yet implemented "<< endl;

}

bool UMLEntityConstraint::showPropertiesDialog(QWidget* parent) {
      kDebug() << k_funcinfo << "Not Yet implemented "<< endl;

}

bool UMLEntityConstraint::load( QDomElement & element ) {
      kDebug() << k_funcinfo << "Not Yet implemented "<< endl;

}

QString UMLEntityConstraint::toString(Uml::Signature_Type sig ) {
      kDebug() << k_funcinfo << "Not Yet implemented "<< endl;

}

void UMLEntityConstraint::copyInto(UMLEntityConstraint *rhs) const {

    // call the parent first.
    UMLClassifierListItem::copyInto(rhs);

}

#include "entityconstraint.moc"


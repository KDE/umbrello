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
#include "checkconstraint.h"

// qt/kde includes
#include <kdebug.h>

#include "dialogs/umlcheckconstraintdialog.h"


UMLCheckConstraint::UMLCheckConstraint(const UMLObject *parent,
                          const QString& name, Uml::IDType id)
    : UMLEntityConstraint(parent, name, id) {
    init();
}

UMLCheckConstraint::UMLCheckConstraint(const UMLObject *parent)
    : UMLEntityConstraint( parent ) {
    init();
}

bool UMLCheckConstraint::operator==( UMLCheckConstraint &rhs) {
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    return true;
}

UMLCheckConstraint::~UMLCheckConstraint() { }

void UMLCheckConstraint::copyInto(UMLCheckConstraint *rhs) const {

    // call the parent first.
    UMLEntityConstraint::copyInto(rhs);

    // Copy all datamembers
    rhs->m_CheckCondition = m_CheckCondition;

}

UMLObject* UMLCheckConstraint::clone() const {
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLCheckConstraint *clone = new UMLCheckConstraint( static_cast<UMLObject*>(parent()) );
    copyInto(clone);
    return clone;
}

QString UMLCheckConstraint::toString(Uml::Signature_Type sig ) {
    QString s;

    if(sig == Uml::st_ShowSig || sig == Uml::st_ShowSig || sig == Uml::st_SigNoVis) {
        s = getName() ;
    }

    return s;
}

QString UMLCheckConstraint::getFullyQualifiedName(const QString& separator,
                                                   bool includeRoot ) const {

    kDebug()<< k_funcinfo <<"Nothing implemented yet";
    return this->getName();
}

void UMLCheckConstraint::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement checkConstraintElement = UMLObject::save("UML:CheckConstraint", qDoc);

    QDomNode checkCondition  = qDoc.createTextNode( m_CheckCondition );
    checkConstraintElement.appendChild( checkCondition );

    qElement.appendChild( checkConstraintElement );
}

bool UMLCheckConstraint::showPropertiesDialog(QWidget* parent) {
    UMLCheckConstraintDialog dialog(parent, this);
    return dialog.exec();
}

bool UMLCheckConstraint::load( QDomElement & element ) {

    QDomNode node = element.firstChild();

    QDomText checkConstraintText = node.toText();
    if ( checkConstraintText.isNull() )
        return false;

    m_CheckCondition = checkConstraintText.data();

    return true;
}


void UMLCheckConstraint::init() {

    m_BaseType = Uml::ot_CheckConstraint;
    m_CheckCondition = QString();
}

#include "checkconstraint.moc"



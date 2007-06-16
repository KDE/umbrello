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
#include "uniqueconstraint.h"
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "entity.h"
#include "entityattribute.h"
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"
#include "dialogs/umlattributedialog.h"
#include "dialogs/umluniqueconstraintdialog.h"
#include "object_factory.h"


UMLUniqueConstraint::UMLUniqueConstraint(const UMLObject *parent,
                          const QString& name, Uml::IDType id)
    : UMLEntityConstraint(parent, name, id) {
    init();
}

UMLUniqueConstraint::UMLUniqueConstraint(const UMLObject *parent)
    : UMLEntityConstraint( parent ) {
    init();
}

bool UMLUniqueConstraint::operator==( UMLUniqueConstraint &rhs) {
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    return true;
}

UMLUniqueConstraint::~UMLUniqueConstraint() { }

void UMLUniqueConstraint::copyInto(UMLUniqueConstraint *rhs) const {

    // call the parent first.
    UMLEntityConstraint::copyInto(rhs);


    // Copy all datamembers
    rhs->m_EntityAttributeList.clear();
    bool valid = true;
    foreach( UMLEntityAttribute* attr, m_EntityAttributeList ) {
       if ( !valid )
           break;
       valid = rhs->addEntityAttribute( attr );
    }

    if ( !valid ) {
        rhs->m_EntityAttributeList.clear();
        kDebug() << k_funcinfo <<"Copying Attributes Failed : Rhs List cleared instead"<<endl;
    }
}

UMLObject* UMLUniqueConstraint::clone() const {
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLUniqueConstraint *clone = new UMLUniqueConstraint( static_cast<UMLObject*>(parent()) );
    copyInto(clone);
    return clone;
}

QString UMLUniqueConstraint::toString(Uml::Signature_Type sig ) {
     QString s;
    //FIXME

    if(sig == Uml::st_ShowSig || sig == Uml::st_ShowSig || sig == Uml::st_SigNoVis) {
        s = getName() + ':';

        if ( static_cast<UMLEntity*>( parent() )->isPrimaryKey( this ) ) {
           s += "Primary Key (";
        } else {
           s += "Unique (";
        }

        bool first = true;
        foreach( UMLEntityAttribute* att, m_EntityAttributeList ) {
            if ( first ) {
               first = false;
            } else
                s += ' , ';
            s += att->getName();
        }
        s +=  ')' ;
    }

    return s;
}

QString UMLUniqueConstraint::getFullyQualifiedName(QString separator,
                                                   bool includeRoot ) const {

    kDebug()<< k_funcinfo <<"Nothing implemented yet";
    return this->getName();
}

void UMLUniqueConstraint::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    kDebug()<< k_funcinfo <<"Nothing implemented yet";
}

bool UMLUniqueConstraint::showPropertiesDialog(QWidget* parent) {
    UMLUniqueConstraintDialog dialog(parent, this);
    return dialog.exec();
}

bool UMLUniqueConstraint::load( QDomElement & element ) {
    kDebug()<< k_funcinfo <<"Nothing implemented yet"<<endl;
    return true;
}


bool UMLUniqueConstraint::hasEntityAttribute(UMLEntityAttribute* attr) {

    if ( m_EntityAttributeList.findRef( attr ) == -1 ) {
        //not present
        return false;
    }

    // else present
    return true;

}

bool UMLUniqueConstraint::addEntityAttribute(UMLEntityAttribute* attr) {

    UMLEntity *owningParent = dynamic_cast<UMLEntity*>(parent());

    if ( hasEntityAttribute( attr ) ) {
        kDebug() << k_funcinfo << "Unique Constraint already contains"<<attr->getName()<<endl;
        return false;

    }
    if (owningParent == NULL) {
        kError() << k_funcinfo << m_Name
        << "): parent " << owningParent->getName()
        << " is not a UMLEntity" << endl;
        return false;
    }

    if ( owningParent->findChildObjectById( attr->getID() ) == NULL ) {
        kError() << k_funcinfo
        << " parent " << owningParent->getName()
                 << " does not contain attribute " << attr->getName()<<endl;
        return false;
    }

    //else add the attribute to the Entity Attribute List
    m_EntityAttributeList.append( attr );

    return true;

}

bool UMLUniqueConstraint::removeEntityAttribute(UMLEntityAttribute* attr) {

    UMLEntity *owningParent = dynamic_cast<UMLEntity*>(parent());

    if (owningParent == NULL) {
        kError() << k_funcinfo << m_Name
        << "): parent " << owningParent->getName()
        << " is not a UMLEntity" << endl;
        return false;
    }

    /*
     * The attribute may already be removed from the Entity when this function
     * is called. So checking this is not right
     *
     * if ( owningParent->findChildObjectById( attr->getID() ) == NULL ) {
     *    kError() << k_funcinfo
     *    << " parent " << owningParent->getName()
     *             << " does not contain attribute " << attr->getName()<<endl;
     *    return false;
     * }
     */

    //else remove the attribute from the Entity Attribute List
    if ( m_EntityAttributeList.remove( attr ) ) {
        return true;
    }

    return false;

}

void UMLUniqueConstraint::init(){

    m_BaseType = Uml::ot_UniqueConstraint;

}

void UMLUniqueConstraint::clearAttributeList(){
    m_EntityAttributeList.clear();
}




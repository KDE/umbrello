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
#include "foreignkeyconstraint.h"
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "entity.h"
#include "entityattribute.h"
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"
#include "dialogs/umlforeignkeyconstraintdialog.h"
#include "object_factory.h"

UMLForeignKeyConstraint::UMLForeignKeyConstraint(const UMLObject *parent,
    const QString& name, Uml::IDType id)
    : UMLEntityConstraint(parent, name, id) {
    init();
}

UMLForeignKeyConstraint::UMLForeignKeyConstraint(const UMLObject *parent)
    : UMLEntityConstraint( parent ) {
    init();
}

void UMLForeignKeyConstraint::init() {
    // initialise attributes
     m_BaseType = Uml::ot_ForeignKeyConstraint;

     // should be NULL actually
     // self referencing assigned to protect default behaviour
     m_ReferencedEntity = static_cast<UMLEntity*>( parent() );

     m_UpdateAction = uda_NoAction;
     m_DeleteAction = uda_NoAction;

    // connecte signals and slots
     connect( this,SIGNAL( sigReferencedEntityChanged() ),this,SLOT( slotReferencedEntityChanged() ) );
}

bool UMLForeignKeyConstraint::operator==( UMLForeignKeyConstraint &rhs) {
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    return true;

}

UMLForeignKeyConstraint::~UMLForeignKeyConstraint() {}

void UMLForeignKeyConstraint::copyInto(UMLForeignKeyConstraint *rhs) const {

    // call the parent first.
    UMLEntityConstraint::copyInto(rhs);

    // Copy all datamembers
    rhs->m_ReferencedEntity = m_ReferencedEntity;
    rhs->m_AttributeMap = m_AttributeMap;
    rhs->m_DeleteAction = m_DeleteAction;
    rhs->m_UpdateAction = m_UpdateAction;
}

UMLObject* UMLForeignKeyConstraint::clone() const {
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLForeignKeyConstraint *clone = new UMLForeignKeyConstraint( static_cast<UMLObject*>(parent()) );
    copyInto(clone);
    return clone;
}

QString UMLForeignKeyConstraint::toString(Uml::Signature_Type sig ){

    QString s;

    if(sig == Uml::st_ShowSig || sig == Uml::st_ShowSig || sig == Uml::st_SigNoVis) {
        s = getName() + ':';
        s += " Foreign Key (";
        QList<UMLEntityAttribute*> keys = m_AttributeMap.keys();
        bool first = true;
        foreach( UMLEntityAttribute* key, keys ) {
            if ( first ) {
                first = false;
            } else
                s += ',';
            s += key->getName();
        }
        s += ')';
    }

    return s;
}


void UMLForeignKeyConstraint::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    kDebug()<< k_funcinfo <<"Nothing implemented yet"<<endl;
}

bool UMLForeignKeyConstraint::showPropertiesDialog(QWidget* parent) {
    UMLForeignKeyConstraintDialog dialog(parent, this );
    return dialog.exec();
}

bool UMLForeignKeyConstraint::addEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr) {


    UMLEntity *owningParent = dynamic_cast<UMLEntity*>(parent());

    if ( pAttr == NULL or rAttr == NULL ) {
        kError()<< k_funcinfo <<"null values passed to function"<<endl;
        return false;
    }
    // check for sanity of pAttr ( parent entity attribute )
    if (owningParent == NULL) {
        kError() << k_funcinfo << m_Name
        << "): parent " << owningParent->getName()
        << " is not a UMLEntity" << endl;
        return false;
    }

    if ( owningParent->findChildObjectById( pAttr->getID() ) == NULL ) {
        kError() << k_funcinfo
        << " parent " << owningParent->getName()
                 << " does not contain attribute " << pAttr->getName()<<endl;
        return false;
    }

    //check for sanity of rAttr ( referenced entity attribute )
    if ( m_ReferencedEntity != NULL ) {
       if ( m_ReferencedEntity->findChildObjectById( rAttr->getID() ) == NULL ) {
        kError() << k_funcinfo
        << " parent " << m_ReferencedEntity->getName()
                 << " does not contain attribute " << rAttr->getName()<<endl;
        return false;
       }
    } else {
        kError()<< k_funcinfo << "Referenced Table Not set. Not Adding Pair "<< endl;
        return false;
    }

    // check if key takes part in some mapping
    if ( m_AttributeMap.contains( pAttr ) == true )
        return false;

    // check if value takes part in some mapping ( no direct function)
    foreach( UMLEntityAttribute* attr, m_AttributeMap.values() ) {
        if ( rAttr == attr )
            return false;
    }

    // passed all checks, insert now
    m_AttributeMap.insert(pAttr, rAttr);

     QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
     for (i = m_AttributeMap.begin(); i != m_AttributeMap.end(); ++i)
         kDebug()<<i.key()->getName()<<" "<<i.key()->getBaseType()
                 <<" "<<i.value()->getName()<<" "<<i.value()->getBaseType()<<endl;


     return true;
}

bool UMLForeignKeyConstraint::removeEntityAttributePair(UMLEntityAttribute* /*key*/ pAttr) {

    bool state = m_AttributeMap.remove( pAttr );

    return state;

}

bool UMLForeignKeyConstraint::hasEntityAttributePair(UMLEntityAttribute* pAttr,UMLEntityAttribute* rAttr) {

    if ( m_AttributeMap.contains( pAttr ) ) {
        if ( m_AttributeMap.value( pAttr ) == rAttr ) {
            return true;
        }
    }

    return false;
}

bool UMLForeignKeyConstraint::load( QDomElement & element ) {
     kDebug()<< k_funcinfo <<"Nothing implemented yet";
     return true;
}



void UMLForeignKeyConstraint::setReferencedEntity(UMLEntity* ent){
    if ( ent == m_ReferencedEntity )
        return;

    m_ReferencedEntity = ent;

    emit sigReferencedEntityChanged();

}

void UMLForeignKeyConstraint::slotReferencedEntityChanged(){

    // clear all mappings
    m_AttributeMap.clear();
}

void UMLForeignKeyConstraint::clearMappings(){
    m_AttributeMap.clear();

}

#include "foreignkeyconstraint.moc"




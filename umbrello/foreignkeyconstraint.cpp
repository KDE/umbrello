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
#include <qpair.h>
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
    QDomElement foreignKeyConstraintElement = UMLObject::save( "UML:ForeignKeyConstraint", qDoc );

    foreignKeyConstraintElement.setAttribute( "referencedEntity", ID2STR( m_ReferencedEntity->getID() ) );

    int updateAction = (int)m_UpdateAction;
    int deleteAction = (int)m_DeleteAction;

    foreignKeyConstraintElement.setAttribute( "updateAction", updateAction );
    foreignKeyConstraintElement.setAttribute( "deleteAction", deleteAction );

    QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
    for (i = m_AttributeMap.begin(); i!= m_AttributeMap.end() ; ++i) {
        QDomElement mapElement = qDoc.createElement( "AttributeMap" );
        mapElement.setAttribute( "key", ID2STR((i.key())->getID()) );
        mapElement.setAttribute( "value", ID2STR((i.value())->getID()) );
        foreignKeyConstraintElement.appendChild( mapElement );
    }

    qElement.appendChild(foreignKeyConstraintElement);
}

bool UMLForeignKeyConstraint::showPropertiesDialog(QWidget* parent) {
    UMLForeignKeyConstraintDialog dialog(parent, this );
    return dialog.exec();
}

bool UMLForeignKeyConstraint::addEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr) {


    UMLEntity *owningParent = dynamic_cast<UMLEntity*>(parent());

    if ( pAttr == NULL || rAttr == NULL ) {
        kError()<<"null values passed to function"<<endl;
        return false;
    }
    // check for sanity of pAttr ( parent entity attribute )
    if (owningParent == NULL) {
        kError() << m_Name
        << "): parent " << owningParent->getName()
        << " is not a UMLEntity" << endl;
        return false;
    }

    if ( owningParent->findChildObjectById( pAttr->getID() ) == NULL ) {
        kError() 
        << " parent " << owningParent->getName()
                 << " does not contain attribute " << pAttr->getName()<<endl;
        return false;
    }

    //check for sanity of rAttr ( referenced entity attribute )
    if ( m_ReferencedEntity != NULL ) {
       if ( m_ReferencedEntity->findChildObjectById( rAttr->getID() ) == NULL ) {
        kError() 
        << " parent " << m_ReferencedEntity->getName()
                 << " does not contain attribute " << rAttr->getName()<<endl;
        return false;
       }
    } else {
        kError()<< "Referenced Table Not set. Not Adding Pair "<< endl;
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

bool UMLForeignKeyConstraint::hasEntityAttributePair(UMLEntityAttribute* pAttr,UMLEntityAttribute* rAttr) const {

    if ( m_AttributeMap.contains( pAttr ) ) {
        if ( m_AttributeMap.value( pAttr ) == rAttr ) {
            return true;
        }
    }

    return false;
}

bool UMLForeignKeyConstraint::load( QDomElement & element ) {
    UMLDoc* doc = UMLApp::app()->getDocument();

    Uml::IDType referencedEntityId = STR2ID( element.attribute("referencedEntity","" ) );

    UMLObject* obj = doc->findObjectById(referencedEntityId);
    m_ReferencedEntity = static_cast<UMLEntity*>(obj);

    if ( m_ReferencedEntity == NULL ) {
        // save for resolving later
        m_pReferencedEntityID = referencedEntityId;
    }

    m_UpdateAction = (UpdateDeleteAction)element.attribute( "updateAction" ).toInt();
    m_DeleteAction = (UpdateDeleteAction)element.attribute( "deleteAction" ).toInt();

    QDomNode node = element.firstChild();
    while ( !node.isNull() ) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (Uml::tagEq(tag, "AttributeMap")) {

            Uml::IDType keyId = STR2ID(tempElement.attribute("key","" ));
            Uml::IDType valueId = STR2ID(tempElement.attribute("value","" ));

            UMLEntityAttribute* key = NULL , *value = NULL;

            UMLEntity* parentEntity = static_cast<UMLEntity*>( parent() );
            UMLObject* keyObj = parentEntity->findChildObjectById(keyId);
            key = static_cast<UMLEntityAttribute*>(keyObj);

            if ( m_ReferencedEntity == NULL ) {
                // if referenced entity is null, then we won't find its attributes even
                // save for resolving later
                m_pEntityAttributeIDMap.insert( key, valueId );
            } else {
               UMLObject* valueObj = m_ReferencedEntity->findChildObjectById(valueId);
               value = static_cast<UMLEntityAttribute*>( valueObj );
            }


        } else {
            kWarning() << "unknown child type in UMLUniqueConstraint::load";
        }

        node = node.nextSibling();
    }

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

bool UMLForeignKeyConstraint::resolveRef() {
    // resolve referenced entity first
    UMLDoc* doc = UMLApp::app()->getDocument();

    bool success = true;

    //resolve the referenced entity
    if ( !ID2STR(m_pReferencedEntityID).isEmpty() ) {
        UMLObject* obj = doc->findObjectById(m_pReferencedEntityID);
        m_ReferencedEntity = static_cast<UMLEntity*>(obj);
        if (m_ReferencedEntity == NULL ) {
            success = false;
        }
    }

    QMap<UMLEntityAttribute*, Uml::IDType>::iterator i;
    for (i = m_pEntityAttributeIDMap.begin(); i!= m_pEntityAttributeIDMap.end() ; ++i) {
       if ( !ID2STR(i.value()).isEmpty() ) {
           UMLObject* obj = doc->findObjectById(i.value());
           m_AttributeMap[i.key()] = static_cast<UMLEntityAttribute*>(obj);
           if ( m_AttributeMap[i.key()] == NULL ) {
               success = false;
           }
       }
    }

    return success;
}

#include "foreignkeyconstraint.moc"




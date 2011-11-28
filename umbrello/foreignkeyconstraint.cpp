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
#include "foreignkeyconstraint.h"

// app includes
#include "debug_utils.h"
#include "entity.h"
#include "entityattribute.h"
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"
#include "umlforeignkeyconstraintdialog.h"
#include "object_factory.h"

/**
 * Sets up a constraint.
 * @param parent    The parent of this UMLForeignKeyConstraint.
 * @param name      The name of this UMLForeignKeyConstraint.
 * @param id        The unique id given to this UMLForeignKeyConstraint.
 */
UMLForeignKeyConstraint::UMLForeignKeyConstraint(UMLObject *parent,
    const QString& name, Uml::IDType id)
  : UMLEntityConstraint(parent, name, id)
{
    init();
}

/**
 * Sets up a constraint.
 * @param parent    The parent of this UMLForeignKeyConstraint.
 */
UMLForeignKeyConstraint::UMLForeignKeyConstraint(UMLObject *parent)
    : UMLEntityConstraint( parent )
{
    init();
}

/**
 * Initialisation of common variables
 */
void UMLForeignKeyConstraint::init()
{
    // initialise attributes
     m_BaseType = UMLObject::ot_ForeignKeyConstraint;

     // should be NULL actually
     // self referencing assigned to protect default behaviour
     m_ReferencedEntity = static_cast<UMLEntity*>( parent() );

     m_UpdateAction = uda_NoAction;
     m_DeleteAction = uda_NoAction;

    // connecte signals and slots
     connect(this, SIGNAL(sigReferencedEntityChanged()), this, SLOT(slotReferencedEntityChanged()));
}

/**
 * Overloaded '==' operator
 */
bool UMLForeignKeyConstraint::operator==( const UMLForeignKeyConstraint &rhs) const
{
    if( this == &rhs )
        return true;

    if( !UMLObject::operator==( rhs ) )
        return false;

    return true;
}

/**
 * Destructor.
 */
UMLForeignKeyConstraint::~UMLForeignKeyConstraint()
{
}

/**
 * Copy the internal presentation of this object into the UMLForeignKeyConstraint
 * object.
 */
void UMLForeignKeyConstraint::copyInto(UMLObject *lhs) const
{
    UMLForeignKeyConstraint *target = static_cast<UMLForeignKeyConstraint*>(lhs);

    // call the parent first.
    UMLEntityConstraint::copyInto(target);

    // Copy all datamembers
    target->m_ReferencedEntity = m_ReferencedEntity;
    target->m_AttributeMap = m_AttributeMap;
    target->m_DeleteAction = m_DeleteAction;
    target->m_UpdateAction = m_UpdateAction;
}

/**
 * Make a clone of the UMLForeignKeyConstraint.
 */
UMLObject* UMLForeignKeyConstraint::clone() const
{
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLForeignKeyConstraint *clone = new UMLForeignKeyConstraint( static_cast<UMLObject*>(parent()) );
    copyInto(clone);
    return clone;
}

/**
 * Returns a string representation of the UMLForeignKeyConstraint.
 * @param sig   If true will show the attribute type and initial value.
 * @return  Returns a string representation of the UMLAttribute.
 */
QString UMLForeignKeyConstraint::toString(Uml::SignatureType sig)
{
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        s = name() + ':';
        s += " Foreign Key (";
        QList<UMLEntityAttribute*> keys = m_AttributeMap.keys();
        bool first = true;
        foreach( UMLEntityAttribute* key, keys ) {
            if ( first ) {
                first = false;
            } else
                s += ',';
            s += key->name();
        }
        s += ')';
    }

    return s;
}

/**
 * Creates the <UML:ForeignKeyConstraint> XMI element.
 */
void UMLForeignKeyConstraint::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    QDomElement foreignKeyConstraintElement = UMLObject::save( "UML:ForeignKeyConstraint", qDoc );

    foreignKeyConstraintElement.setAttribute( "referencedEntity", ID2STR( m_ReferencedEntity->id() ) );

    int updateAction = (int)m_UpdateAction;
    int deleteAction = (int)m_DeleteAction;

    foreignKeyConstraintElement.setAttribute( "updateAction", updateAction );
    foreignKeyConstraintElement.setAttribute( "deleteAction", deleteAction );

    QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
    for (i = m_AttributeMap.begin(); i!= m_AttributeMap.end() ; ++i) {
        QDomElement mapElement = qDoc.createElement( "AttributeMap" );
        mapElement.setAttribute( "key", ID2STR((i.key())->id()) );
        mapElement.setAttribute( "value", ID2STR((i.value())->id()) );
        foreignKeyConstraintElement.appendChild( mapElement );
    }

    qElement.appendChild(foreignKeyConstraintElement);
}

/**
 * Display the properties configuration dialog for the attribute.
 */
bool UMLForeignKeyConstraint::showPropertiesDialog(QWidget* parent)
{
    UMLForeignKeyConstraintDialog dialog(parent, this );
    return dialog.exec();
}

/**
 * Adds the attribute pair to the attributeMap
 * @param pAttr The Attribute of the Parent Entity
 * @param rAttr The Attribute of the Referenced Entity
 * @return true if the attribute pair could be added successfully
 */
bool UMLForeignKeyConstraint::addEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr)
{
    UMLEntity *owningParent = dynamic_cast<UMLEntity*>(parent());

    if ( pAttr == NULL || rAttr == NULL ) {
        uError() << "null values passed to function";
        return false;
    }
    // check for sanity of pAttr ( parent entity attribute )
    if (owningParent == NULL) {
        uError() << name() << ": parent is not a UMLEntity";
        return false;
    }

    if ( owningParent->findChildObjectById( pAttr->id() ) == NULL ) {
        uError() << " parent " << owningParent->name()
                 << " does not contain attribute " << pAttr->name();
        return false;
    }

    //check for sanity of rAttr ( referenced entity attribute )
    if ( m_ReferencedEntity != NULL ) {
       if ( m_ReferencedEntity->findChildObjectById( rAttr->id() ) == NULL ) {
        uError() << " parent " << m_ReferencedEntity->name()
                 << " does not contain attribute " << rAttr->name();
        return false;
       }
    } else {
        uError() << "Referenced Table Not set. Not Adding Pair ";
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
         uDebug() << i.key()->name() << " " << i.key()->baseType()
                 << " " << i.value()->name() << " " << i.value()->baseType();

     return true;
}

/**
 * Removes an Attribute pair
 * @param pAttr The Attribute of the Parent Entity in the map. This attribute is the
                key of the map.
 * @return true of the attribute pair could be removed successfully
 */
bool UMLForeignKeyConstraint::removeEntityAttributePair(UMLEntityAttribute* /*key*/ pAttr)
{
    bool state = m_AttributeMap.remove( pAttr );

    return state;
}

/**
 * Check if a attribute pair already exists
 * @param pAttr The Attribute of the Parent Entity
 * @param rAttr The Attribute of the Referenced Entity
 * @return true if the attribute pair could be found.
 */
bool UMLForeignKeyConstraint::hasEntityAttributePair(UMLEntityAttribute* pAttr,UMLEntityAttribute* rAttr) const
{
    if ( m_AttributeMap.contains( pAttr ) ) {
        if ( m_AttributeMap.value( pAttr ) == rAttr ) {
            return true;
        }
    }

    return false;
}

/**
 * Loads the <UML:ForeignKeyConstraint> XMI element.
 */
bool UMLForeignKeyConstraint::load( QDomElement & element )
{
    UMLDoc* doc = UMLApp::app()->document();

    Uml::IDType referencedEntityId = STR2ID( element.attribute("referencedEntity", "") );

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
        if (UMLDoc::tagEq(tag, "AttributeMap")) {

            Uml::IDType keyId = STR2ID(tempElement.attribute("key", ""));
            Uml::IDType valueId = STR2ID(tempElement.attribute("value", ""));

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
            uWarning() << "unknown child type in UMLUniqueConstraint::load";
        }

        node = node.nextSibling();
    }

    return true;
}

/**
 * Set the Referenced Entity.
 * @param ent The Entity to Reference
 */
void UMLForeignKeyConstraint::setReferencedEntity(UMLEntity* ent)
{
    if ( ent == m_ReferencedEntity )
        return;

    m_ReferencedEntity = ent;

    emit sigReferencedEntityChanged();
}

/**
 * Get the Referenced Entity.
 * @return the UML entity object
 */
UMLEntity* UMLForeignKeyConstraint::getReferencedEntity() const
{
    return m_ReferencedEntity;
}

/**
 * Slot for referenced entity changed.
 */
void UMLForeignKeyConstraint::slotReferencedEntityChanged()
{
    // clear all mappings
    m_AttributeMap.clear();
}

/**
 * Clears all mappings between local and referenced attributes
 */
void UMLForeignKeyConstraint::clearMappings()
{
    m_AttributeMap.clear();
}

/**
 * Remimplementation from base classes
 * Used to resolve forward references to referenced entities in xmi
 */
bool UMLForeignKeyConstraint::resolveRef()
{
    // resolve referenced entity first
    UMLDoc* doc = UMLApp::app()->document();

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

/**
 * Retrieve all Pairs of Attributes.
 */
QMap<UMLEntityAttribute*,UMLEntityAttribute*> UMLForeignKeyConstraint::getEntityAttributePairs()
{
    return m_AttributeMap;
}

/**
 * Get the Delete Action.
 */
UMLForeignKeyConstraint::UpdateDeleteAction UMLForeignKeyConstraint::getDeleteAction() const
{
    return m_DeleteAction;
}

/**
 * Get the Update Action.
 */
UMLForeignKeyConstraint::UpdateDeleteAction UMLForeignKeyConstraint::getUpdateAction() const
{
    return m_UpdateAction;
}

/**
 * Set the Delete Action to the specified UpdateDeleteAction.
 */
void UMLForeignKeyConstraint::setDeleteAction(UpdateDeleteAction uda)
{
    m_DeleteAction = uda;
}

/**
 * Set the Update Action to the specified UpdateDeleteAction
 */
void UMLForeignKeyConstraint::setUpdateAction(UpdateDeleteAction uda)
{
    m_UpdateAction = uda;
}

#include "foreignkeyconstraint.moc"

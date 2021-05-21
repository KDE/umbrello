/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
    const QString& name, Uml::ID::Type id)
  : UMLEntityConstraint(parent, name, id)
{
    init();
}

/**
 * Sets up a constraint.
 * @param parent    The parent of this UMLForeignKeyConstraint.
 */
UMLForeignKeyConstraint::UMLForeignKeyConstraint(UMLObject *parent)
    : UMLEntityConstraint(parent)
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
     m_ReferencedEntity = umlParent()->asUMLEntity();

     m_UpdateAction = uda_NoAction;
     m_DeleteAction = uda_NoAction;

    // connect signals and slots
     connect(this, SIGNAL(sigReferencedEntityChanged()), this, SLOT(slotReferencedEntityChanged()));
}

/**
 * Overloaded '==' operator
 */
bool UMLForeignKeyConstraint::operator==(const UMLForeignKeyConstraint &rhs) const
{
    if(this == &rhs)
        return true;

    if(!UMLObject::operator==(rhs))
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
    UMLForeignKeyConstraint *target = lhs->asUMLForeignKeyConstraint();

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
    UMLForeignKeyConstraint *clone = new UMLForeignKeyConstraint(umlParent());
    copyInto(clone);
    return clone;
}

/**
 * Returns a string representation of the UMLForeignKeyConstraint.
 * @param sig   If true will show the attribute type and initial value.
 * @return  Returns a string representation of the UMLAttribute.
 */
QString UMLForeignKeyConstraint::toString(Uml::SignatureType::Enum sig, bool withStereotype)
{
    Q_UNUSED(withStereotype);
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        s = name() + QLatin1Char(':');
        s += QLatin1String(" Foreign Key (");
        QList<UMLEntityAttribute*> keys = m_AttributeMap.keys();
        bool first = true;
        foreach(UMLEntityAttribute* key, keys) {
            if (first) {
                first = false;
            } else
                s += QLatin1Char(',');
            s += key->name();
        }
        s += QLatin1Char(')');
    }

    return s;
}

/**
 * Creates the <UML:ForeignKeyConstraint> XMI element.
 */
void UMLForeignKeyConstraint::saveToXMI1(QXmlStreamWriter& writer)
{
    UMLObject::save1(QLatin1String("UML:ForeignKeyConstraint"), writer);

    writer.writeAttribute(QLatin1String("referencedEntity"), Uml::ID::toString(m_ReferencedEntity->id()));

    int updateAction = (int)m_UpdateAction;
    int deleteAction = (int)m_DeleteAction;

    writer.writeAttribute(QLatin1String("updateAction"), QString::number(updateAction));
    writer.writeAttribute(QLatin1String("deleteAction"), QString::number(deleteAction));

    QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
    for (i = m_AttributeMap.begin(); i!= m_AttributeMap.end() ; ++i) {
        writer.writeStartElement(QLatin1String("AttributeMap"));
        writer.writeAttribute(QLatin1String("key"), Uml::ID::toString((i.key())->id()));
        writer.writeAttribute(QLatin1String("value"), Uml::ID::toString((i.value())->id()));
        writer.writeEndElement();
    }

    UMLObject::save1end(writer);
}

/**
 * Display the properties configuration dialog for the attribute.
 */
bool UMLForeignKeyConstraint::showPropertiesDialog(QWidget* parent)
{
    UMLForeignKeyConstraintDialog dialog(parent, this);
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
    UMLEntity *owningParent = umlParent()->asUMLEntity();

    if (pAttr == 0 || rAttr == 0) {
        uError() << "null values passed to function";
        return false;
    }
    // check for sanity of pAttr (parent entity attribute)
    if (owningParent == 0) {
        uError() << name() << ": parent is not a UMLEntity";
        return false;
    }

    if (owningParent->findChildObjectById(pAttr->id()) == 0) {
        uError() << " parent " << owningParent->name()
                 << " does not contain attribute " << pAttr->name();
        return false;
    }

    //check for sanity of rAttr (referenced entity attribute)
    if (m_ReferencedEntity != 0) {
       if (m_ReferencedEntity->findChildObjectById(rAttr->id()) == 0) {
        uError() << " parent " << m_ReferencedEntity->name()
                 << " does not contain attribute " << rAttr->name();
        return false;
       }
    } else {
        uError() << "Referenced Table Not set. Not Adding Pair ";
        return false;
    }

    // check if key takes part in some mapping
    if (m_AttributeMap.contains(pAttr) == true)
        return false;

    // check if value takes part in some mapping (no direct function)
    foreach(UMLEntityAttribute* attr, m_AttributeMap.values()) {
        if (rAttr == attr)
            return false;
    }

    // passed all checks, insert now
    m_AttributeMap.insert(pAttr, rAttr);

     QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
     for (i = m_AttributeMap.begin(); i != m_AttributeMap.end(); ++i)
         uDebug() << i.key()->name() << QLatin1String(" ") << i.key()->baseType()
                 << QLatin1String(" ") << i.value()->name() << QLatin1String(" ") << i.value()->baseType();

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
    bool state = m_AttributeMap.remove(pAttr);

    return state;
}

/**
 * Check if an attribute pair already exists
 * @param pAttr The Attribute of the Parent Entity
 * @param rAttr The Attribute of the Referenced Entity
 * @return true if the attribute pair could be found.
 */
bool UMLForeignKeyConstraint::hasEntityAttributePair(UMLEntityAttribute* pAttr, UMLEntityAttribute* rAttr) const
{
    if (m_AttributeMap.contains(pAttr)) {
        if (m_AttributeMap.value(pAttr) == rAttr) {
            return true;
        }
    }

    return false;
}

/**
 * Loads the <UML:ForeignKeyConstraint> XMI element.
 */
bool UMLForeignKeyConstraint::load1(QDomElement & element)
{
    UMLDoc* doc = UMLApp::app()->document();

    Uml::ID::Type referencedEntityId = Uml::ID::fromString(element.attribute(QLatin1String("referencedEntity")));

    UMLObject* obj = doc->findObjectById(referencedEntityId);
    m_ReferencedEntity = obj->asUMLEntity();

    if (m_ReferencedEntity == 0) {
        // save for resolving later
        m_pReferencedEntityID = referencedEntityId;
    }

    m_UpdateAction = (UpdateDeleteAction)element.attribute(QLatin1String("updateAction")).toInt();
    m_DeleteAction = (UpdateDeleteAction)element.attribute(QLatin1String("deleteAction")).toInt();

    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QLatin1String("AttributeMap"))) {

            QString xmiKey = tempElement.attribute(QLatin1String("key"));
            QString xmiValue = tempElement.attribute(QLatin1String("value"));
            Uml::ID::Type keyId = Uml::ID::fromString(xmiKey);
            Uml::ID::Type valueId = Uml::ID::fromString(xmiValue);

            UMLEntity* parentEntity = umlParent()->asUMLEntity();
            UMLObject* keyObj = parentEntity->findChildObjectById(keyId);
            UMLEntityAttribute* key = keyObj->asUMLEntityAttribute();

            if (keyObj == 0) {
                uWarning() << "unable to resolve foreign key referencing attribute " << xmiKey;
            } else if (m_ReferencedEntity == 0) {
                // if referenced entity is null, then we won't find its attributes even
                // save for resolving later
                m_pEntityAttributeIDMap.insert(key, valueId);
            } else {
                UMLObject* valueObj = m_ReferencedEntity->findChildObjectById(valueId);
                if (valueObj == 0) {
                    uWarning() << "unable to resolve foreign key referenced attribute" << xmiValue;
                } else {
                    m_AttributeMap[key] = valueObj->asUMLEntityAttribute();
                }
            }

        } else {
            uWarning() << "unknown child type in UMLForeignKeyConstraint::load";
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
    if (ent == m_ReferencedEntity)
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
    if (!Uml::ID::toString(m_pReferencedEntityID).isEmpty()) {
        UMLObject* obj = doc->findObjectById(m_pReferencedEntityID);
        m_ReferencedEntity = obj->asUMLEntity();
        if (m_ReferencedEntity == 0) {
            success = false;
        }
    }

    QMap<UMLEntityAttribute*, Uml::ID::Type>::iterator i;
    for (i = m_pEntityAttributeIDMap.begin(); i!= m_pEntityAttributeIDMap.end() ; ++i) {
       if (!Uml::ID::toString(i.value()).isEmpty()) {
           UMLObject* obj = doc->findObjectById(i.value());
           m_AttributeMap[i.key()] = obj->asUMLEntityAttribute();
           if (m_AttributeMap[i.key()] == 0) {
               success = false;
           }
       }
    }

    return success;
}

/**
 * Retrieve all Pairs of Attributes.
 */
QMap<UMLEntityAttribute*, UMLEntityAttribute*> UMLForeignKeyConstraint::getEntityAttributePairs()
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


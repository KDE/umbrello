/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

//own header
#include "umlforeignkeyconstraint.h"

// app includes
#include "debug_utils.h"
#include "umlentity.h"
#include "umlentityattribute.h"
#include "umlobject.h"
#include "umldoc.h"
#include "uml.h"
#include "umlforeignkeyconstraintdialog.h"
#include "object_factory.h"

DEBUG_REGISTER(UMLForeignKeyConstraint)

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
QString UMLForeignKeyConstraint::toString(Uml::SignatureType::Enum sig, bool /*withStereotype*/) const
{
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        s = name() + QLatin1Char(':');
        s += QStringLiteral(" Foreign Key (");
        QList<UMLEntityAttribute*> keys = m_AttributeMap.keys();
        bool first = true;
        for(UMLEntityAttribute* key : keys) {
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
void UMLForeignKeyConstraint::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("ForeignKeyConstraint"));

    writer.writeAttribute(QStringLiteral("referencedEntity"), Uml::ID::toString(m_ReferencedEntity->id()));

    int updateAction = (int)m_UpdateAction;
    int deleteAction = (int)m_DeleteAction;

    writer.writeAttribute(QStringLiteral("updateAction"), QString::number(updateAction));
    writer.writeAttribute(QStringLiteral("deleteAction"), QString::number(deleteAction));

    QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
    for (i = m_AttributeMap.begin(); i!= m_AttributeMap.end() ; ++i) {
        writer.writeStartElement(QStringLiteral("AttributeMap"));
        writer.writeAttribute(QStringLiteral("key"), Uml::ID::toString((i.key())->id()));
        writer.writeAttribute(QStringLiteral("value"), Uml::ID::toString((i.value())->id()));
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

    if (pAttr == nullptr || rAttr == nullptr) {
        logError0("UMLForeignKeyConstraint::addEntityAttributePair: null value passed to function");
        return false;
    }
    // check for sanity of pAttr (parent entity attribute)
    if (owningParent == nullptr) {
        logError1("UMLForeignKeyConstraint::addEntityAttributePair(%1) : parent is not a UMLEntity", name());
        return false;
    }

    if (owningParent->findChildObjectById(pAttr->id()) == nullptr) {
        logError2("UMLForeignKeyConstraint::addEntityAttributePair: parent %1 does not contain attribute %2",
                  owningParent->name(), pAttr->name());
        return false;
    }

    //check for sanity of rAttr (referenced entity attribute)
    if (m_ReferencedEntity != nullptr) {
       if(m_ReferencedEntity->findChildObjectById(rAttr->id()) == nullptr) {
        logError2("UMLForeignKeyConstraint::addEntityAttributePair parent %1 does not contain attribute %2",
                  m_ReferencedEntity->name(), rAttr->name());
        return false;
       }
    } else {
        logError0("UMLForeignKeyConstraint::addEntityAttributePair: Referenced Table Not set. Not Adding Pair");
        return false;
    }

    // check if key takes part in some mapping
    if (m_AttributeMap.contains(pAttr) == true)
        return false;

    // check if value takes part in some mapping (no direct function)
    for(UMLEntityAttribute* attr : m_AttributeMap.values()) {
        if (rAttr == attr)
            return false;
    }

    // passed all checks, insert now
    m_AttributeMap.insert(pAttr, rAttr);

     QMap<UMLEntityAttribute*, UMLEntityAttribute*>::iterator i;
     logDebug0("UMLForeignKeyConstraint::addEntityAttributePair: AttributeMap after insertion "
               "(keyName keyType  valueName valueType)");
     for (i = m_AttributeMap.begin(); i != m_AttributeMap.end(); ++i)
         logDebug4("- %1 %2  %3 %4", i.key()->name(), i.key()->baseType(),
                                     i.value()->name(), i.value()->baseType());
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

    Uml::ID::Type referencedEntityId = Uml::ID::fromString(element.attribute(QStringLiteral("referencedEntity")));

    UMLObject* obj = doc->findObjectById(referencedEntityId);
    m_ReferencedEntity = obj->asUMLEntity();

    if (m_ReferencedEntity == nullptr) {
        // save for resolving later
        m_pReferencedEntityID = referencedEntityId;
    }

    m_UpdateAction = (UpdateDeleteAction)element.attribute(QStringLiteral("updateAction")).toInt();
    m_DeleteAction = (UpdateDeleteAction)element.attribute(QStringLiteral("deleteAction")).toInt();

    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QStringLiteral("AttributeMap"))) {

            QString xmiKey = tempElement.attribute(QStringLiteral("key"));
            QString xmiValue = tempElement.attribute(QStringLiteral("value"));
            Uml::ID::Type keyId = Uml::ID::fromString(xmiKey);
            Uml::ID::Type valueId = Uml::ID::fromString(xmiValue);

            const UMLEntity* parentEntity = umlParent()->asUMLEntity();
            UMLObject* keyObj = parentEntity->findChildObjectById(keyId);
            UMLEntityAttribute* key = keyObj->asUMLEntityAttribute();

            if (keyObj == nullptr) {
                logWarn1("UMLForeignKeyConstraint::load1 unable to resolve foreign key referencing attribute %1",
                         xmiKey);
            } else if (m_ReferencedEntity == nullptr) {
                // if referenced entity is null, then we won't find its attributes even
                // save for resolving later
                m_pEntityAttributeIDMap.insert(key, valueId);
            } else {
                UMLObject* valueObj = m_ReferencedEntity->findChildObjectById(valueId);
                if (valueObj == nullptr) {
                    logWarn1("UMLForeignKeyConstraint::load1 unable to resolve foreign key referenced attribute %1",
                             xmiValue);
                } else {
                    m_AttributeMap[key] = valueObj->asUMLEntityAttribute();
                }
            }

        } else {
            logWarn1("UMLForeignKeyConstraint::load1: unknown child type %1", tag);
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

    Q_EMIT sigReferencedEntityChanged();
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
        if (m_ReferencedEntity == nullptr) {
            success = false;
        }
    }

    QMap<UMLEntityAttribute*, Uml::ID::Type>::iterator i;
    for (i = m_pEntityAttributeIDMap.begin(); i!= m_pEntityAttributeIDMap.end() ; ++i) {
       if (!Uml::ID::toString(i.value()).isEmpty()) {
           UMLObject* obj = doc->findObjectById(i.value());
           m_AttributeMap[i.key()] = obj->asUMLEntityAttribute();
           if (m_AttributeMap[i.key()] == nullptr) {
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


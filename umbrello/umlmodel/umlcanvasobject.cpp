/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlcanvasobject.h"

// local includes
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlclassifier.h"
#include "umlassociation.h"
#include "umlattribute.h"
#include "umloperation.h"
#include "template.h"
#include "stereotype.h"
#include "idchangelog.h"

// kde includes
#include <KLocalizedString>

DEBUG_REGISTER_DISABLED(UMLCanvasObject)

/**
 * Sets up a UMLCanvasObject.
 *
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLCanvasObject::UMLCanvasObject(const QString & name, Uml::ID::Type id)
  : UMLObject(name, id)
{
}

/**
 * Standard deconstructor.
 */
UMLCanvasObject::~UMLCanvasObject()
{
    //removeAllAssociations();
    // No! This is way too late to do that.
    //  It should have been called explicitly before destructing the
    //  UMLCanvasObject.
    if (associations()) {
        DEBUG() << "UMLCanvasObject destructor: FIXME: there are still associations()";
    }
}

/**
 * Return the subset of subordinates that matches the given type.
 *
 * @param assocType   The AssociationType::Enum to match.
 * @return   The list of associations that match assocType.
 */
UMLAssociationList UMLCanvasObject::getSpecificAssocs(Uml::AssociationType::Enum assocType) const
{
    UMLAssociationList list;
    for(UMLObject *o : subordinates()) {
        uIgnoreZeroPointer(o);
        if (o->baseType() != UMLObject::ot_Association)
            continue;
        UMLAssociation *a = o->asUMLAssociation();
        if (a->getAssocType() == assocType)
            list.append(a);
    }
    return list;
}

/**
 * Adds an association end to subordinates.
 *
 * @param assoc  The association to add.
 *               @todo change param type to UMLRole
 */
bool UMLCanvasObject::addAssociationEnd(UMLAssociation* assoc)
{
    Q_ASSERT(assoc);
    // add association only if not already present in list
    if (!hasAssociation(assoc))
    {
        subordinates().append(assoc);

        // Don't emit signals during load from XMI
        UMLObject::emitModified();
        Q_EMIT sigAssociationEndAdded(assoc);
        return true;
    }
    return false;
}

/**
 * Determine if this canvasobject has the given association.
 *
 * @param assoc   The association to check.
 */
bool UMLCanvasObject::hasAssociation(UMLAssociation* assoc) const
{
    uint cnt = subordinates().count(assoc);
    DEBUG() << "count is " << cnt;
    return (cnt > 0);
}

/**
 * Remove an association end from the CanvasObject.
 *
 * @param assoc   The association to remove.
 *                @todo change param type to UMLRole
 */
int UMLCanvasObject::removeAssociationEnd(UMLAssociation * assoc)
{
    if (!hasAssociation(assoc) || !subordinates().removeAll(assoc)) {
        DEBUG() << "cannot find given assoc " << assoc << " in list";
        return -1;
    }
    UMLApp::app()->document()->removeAssociation(assoc, false);
    UMLObject::emitModified();
    Q_EMIT sigAssociationEndRemoved(assoc);
    return subordinates().count();
}

/**
 * Remove all association ends from the CanvasObject.
 */
void UMLCanvasObject::removeAllAssociationEnds()
{
    for(UMLObject *o : subordinates()) {
        uIgnoreZeroPointer(o);
        if (o->baseType() != UMLObject::ot_Association) {
            continue;
        }
        UMLAssociation *assoc = o->asUMLAssociation();
        //umldoc->slotRemoveUMLObject(assoc);
        UMLObject* objA = assoc->getObject(Uml::RoleType::A);
        UMLObject* objB = assoc->getObject(Uml::RoleType::B);
        UMLCanvasObject *roleAObj = objA->asUMLCanvasObject();
        if (roleAObj) {
            roleAObj->removeAssociationEnd(assoc);
        } else if (objA) {
            DEBUG() << name() << ": objA " << objA->name() << " is not a UMLCanvasObject";
        } else {
            DEBUG() << name() << "): objA is NULL";
        }
        UMLCanvasObject *roleBObj = objB->asUMLCanvasObject();
        if (roleBObj) {
            roleBObj->removeAssociationEnd(assoc);
        } else if (objB) {
            DEBUG() << name() << "): objB " << objB->name() << " is not a UMLCanvasObject";
        } else {
            DEBUG() << name() << "): objB is NULL";
        }
    }
}

/**
 * Remove all child objects.
 * Just clear list, objects must be deleted where they were created
 * (or we have bad crashes).
 */
void UMLCanvasObject::removeAllChildObjects()
{
    if (!subordinates().isEmpty()) {
        removeAllAssociationEnds();
        subordinates().clear();
    }
}

/**
 * Returns a name for the new association, operation, template
 * or attribute appended with a number if the default name is
 * taken e.g. new_association, new_association_1 etc.
 *
 * @param type      The object type for which to make a name.
 * @param prefix    Optional prefix to use for the name.
 *                  If not given then uniqChildName() will choose the prefix
 *                  internally based on the object type.
 * @return  Unique name string for the ObjectType given.
 */
QString UMLCanvasObject::uniqChildName(const UMLObject::ObjectType type,
                                       const QString &prefix /* = QString() */) const
{
    QString currentName;
    currentName = prefix;
    if (currentName.isEmpty()) {
        switch (type) {
            case UMLObject::ot_Association:
                currentName = i18n("new_association");
                break;
            case UMLObject::ot_Attribute:
                currentName = i18n("new_attribute");
                break;
            case UMLObject::ot_Template:
                currentName = i18n("new_template");
                break;
            case UMLObject::ot_Operation:
                currentName = i18n("new_operation");
                break;
            case UMLObject::ot_EnumLiteral:
                currentName = i18n("new_literal");
                break;
            case UMLObject::ot_EntityAttribute:
                currentName = i18n("new_field");
                break;
            case UMLObject::ot_UniqueConstraint:
                currentName = i18n("new_unique_constraint");
                break;
            case UMLObject::ot_ForeignKeyConstraint:
                currentName = i18n("new_fkey_constraint");
                break;
            case UMLObject::ot_CheckConstraint:
                currentName = i18n("new_check_constraint");
                break;
            case UMLObject::ot_Instance:
                currentName = i18n("new_object");
            break;
            default:
                logWarn1("UMLCanvasObject::uniqChildName() called for unknown child type %1",
                         UMLObject::toString(type));
                return QStringLiteral("ERROR_in_UMLCanvasObject_uniqChildName");
        }
    }

    QString name = currentName;
    for (int number = 1; findChildObject(name); ++number) {
        name = currentName + QLatin1Char('_') + QString::number(number);
    }
    return name;
}

/**
 * Find a child object with the given name.
 *
 * @param n  The name of the object to find.
 * @param t  The type to find (optional.) If not given then
 *           any object type will match.
 * @return  Pointer to the object found; NULL if none found.
 */
UMLObject * UMLCanvasObject::findChildObject(const QString &n, UMLObject::ObjectType t) const
{
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    for(UMLObject *obj : subordinates()) {
        uIgnoreZeroPointer(obj);
        if (t != UMLObject::ot_UMLObject && obj->baseType() != t)
            continue;
        if (caseSensitive) {
            if (obj->name() == n)
                return obj;
        } else if (obj->name().toLower() == n.toLower()) {
            return obj;
        }
    }
    return nullptr;
}

/**
 * Find an association.
 *
 * @param id        The id of the object to find.
 * @param considerAncestors boolean switch to consider ancestors while searching
 * @return  Pointer to the object found (NULL if not found.)
 */
UMLObject* UMLCanvasObject::findChildObjectById(Uml::ID::Type id, bool considerAncestors) const
{
    Q_UNUSED(considerAncestors);
    for(UMLObject *o : subordinates()) {
        uIgnoreZeroPointer(o);
        if (o->id() == id)
            return o;
    }
    return nullptr;
}

/**
 *  Overloaded '==' operator
 */
bool UMLCanvasObject::operator==(const UMLCanvasObject& rhs) const
{
    if (this == &rhs) {
        return true;
    }
    if (!UMLObject::operator==(rhs)) {
        return false;
    }
    if (subordinates().count() != rhs.subordinates().count()) {
        return false;
    }
    for (int i = 0; i < subordinates().count(); i++) {
        UMLObject *a = subordinates().at(i);
        UMLObject *b = subordinates().at(i);
        if (!(*a == *b))
            return false;
    }
    return true;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLCanvasObject::copyInto(UMLObject *lhs) const
{
    UMLObject::copyInto(lhs);

    // TODO Associations are not copied at the moment. This because
    // the duplicate function (on umlwidgets) do not copy the associations.
    //
    //target->subordinates() = subordinates();
}

/**
 * Returns the number of associations for the CanvasObject.
 * This is the sum of the aggregations and compositions.
 *
 * @return  The number of associations for the Concept.
 */
int UMLCanvasObject::associations() const
{
    int count = 0;
    for(UMLObject *obj : subordinates()) {
        uIgnoreZeroPointer(obj);
        if (obj->baseType() == UMLObject::ot_Association)
            count++;
    }
    return count;
}

/**
 * Return the list of associations for the CanvasObject.
 *
 * @return   The list of associations for the CanvasObject.
 */
UMLAssociationList UMLCanvasObject::getAssociations() const
{
    UMLAssociationList assocs;
    for(UMLObject *o : subordinates()) {
        uIgnoreZeroPointer(o);
        if (o->baseType() != UMLObject::ot_Association)
            continue;
        UMLAssociation *assoc = o->asUMLAssociation();
        assocs.append(assoc);
    }
    return assocs;
}

/**
 * Return a list of the superclasses of this classifier.
 * TODO: This overlaps with UMLClassifier::findSuperClassConcepts(),
 *       see if we can merge the two.
 *
 * @param withRealizations include realizations in the returned list (default=yes)
 * @return  The list of superclasses for the classifier.
 */
UMLClassifierList UMLCanvasObject::getSuperClasses(bool withRealizations) const
{
    UMLClassifierList list;
    UMLAssociationList assocs = getAssociations();
    for(UMLAssociation* a : assocs) {
        uIgnoreZeroPointer(a);
        if ((a->getAssocType() != Uml::AssociationType::Generalization &&
             a->getAssocType() != Uml::AssociationType::Realization) ||
             (!withRealizations && a->getAssocType() == Uml::AssociationType::Realization) ||
                a->getObjectId(Uml::RoleType::A) != id())
            continue;
        UMLClassifier *c = a->getObject(Uml::RoleType::B)->asUMLClassifier();
        if (c) {
            list.append(c);
        } else {
            DEBUG() << name() << ": generalization's other end is not a "
                << "UMLClassifier (id= " << Uml::ID::toString(a->getObjectId(Uml::RoleType::B)) << ")";
        }
    }
    return list;
}

/**
 * Return a list of the classes that inherit from this classifier.
 * TODO: This overlaps with UMLClassifier::findSubClassConcepts(),
 *       see if we can merge the two.
 *
 * @return  The list of classes inheriting from the classifier.
 */
UMLClassifierList UMLCanvasObject::getSubClasses() const
{
    UMLClassifierList list;
    UMLAssociationList assocs = getAssociations();
    for(UMLAssociation* a : assocs) {
        uIgnoreZeroPointer(a);
        if ((a->getAssocType() != Uml::AssociationType::Generalization &&
             a->getAssocType() != Uml::AssociationType::Realization) ||
                a->getObjectId(Uml::RoleType::B) != id())
            continue;
        UMLClassifier *c = a->getObject(Uml::RoleType::A)->asUMLClassifier();
        if (c) {
            list.append(c);
        } else {
            DEBUG() << "specialization's other end is not a UMLClassifier"
                << " (id=" << Uml::ID::toString(a->getObjectId(Uml::RoleType::A)) << ")";
        }
    }
    return list;
}

/**
 * Shorthand for getSpecificAssocs(Uml::at_Realization)
 *
 * @return  The list of realizations for the Concept.
 */
UMLAssociationList UMLCanvasObject::getRealizations() const
{
    return getSpecificAssocs(Uml::AssociationType::Realization);
}

/**
 * Shorthand for getSpecificAssocs(Uml::at_Aggregation)
 *
 * @return  The list of aggregations for the Concept.
 */
UMLAssociationList UMLCanvasObject::getAggregations() const
{
    return getSpecificAssocs(Uml::AssociationType::Aggregation);
}

/**
 * Shorthand for getSpecificAssocs(Uml::at_Composition) const
 *
 * @return  The list of compositions for the Concept.
 */
UMLAssociationList UMLCanvasObject::getCompositions() const
{
    return getSpecificAssocs(Uml::AssociationType::Composition);
}

/**
 * Shorthand for getSpecificAssocs(Uml::at_Relationship)
 *
 * @return  The list of relationships for the entity.
 */
UMLAssociationList UMLCanvasObject::getRelationships() const
{
    return getSpecificAssocs(Uml::AssociationType::Relationship);
}

/**
 * Reimplementation of UMLObject method.
 */
bool UMLCanvasObject::resolveRef()
{
    bool overallSuccess = UMLObject::resolveRef();
    for(UMLObject *obj : subordinates()) {
        uIgnoreZeroPointer(obj);
        if (! obj->resolveRef()) {
            subordinates().removeAll(obj);
            overallSuccess = false;
        }
    }
    return overallSuccess;
}


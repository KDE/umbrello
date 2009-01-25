/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlcanvasobject.h"

// local includes
#include "uml.h"
#include "umldoc.h"
#include "classifier.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"
#include "template.h"
#include "stereotype.h"
#include "clipboard/idchangelog.h"

// kde includes
#include <kdebug.h>
#include <klocale.h>

/**
 * Sets up a UMLCanvasObject.
 *
 * @param name              The name of the Concept.
 * @param id                The unique id of the Concept.
 */
UMLCanvasObject::UMLCanvasObject(const QString & name, Uml::IDType id)
        : UMLObject(name, id)
{
}

UMLCanvasObject::~UMLCanvasObject()
{
    //removeAllAssociations();
    /* No! This is way too late to do that.
      It should have been called explicitly before destructing the
      UMLCanvasObject.
      Here is an example crash that happens if we rely on
      removeAllAssociations() at this point:
#4  0x415aac7f in __dynamic_cast () from /usr/lib/libstdc++.so.5
#5  0x081acdbd in UMLCanvasObject::removeAllAssociations() (this=0x89e5b08)
    at umlcanvasobject.cpp:83
#6  0x081ac9fa in ~UMLCanvasObject (this=0x89e5b08) at umlcanvasobject.cpp:29
#7  0x08193ffc in ~UMLPackage (this=0x89e5b08) at package.cpp:35
#8  0x0813cbf6 in ~UMLClassifier (this=0x89e5b08) at classifier.cpp:40
#9  0x081af3a6 in UMLDoc::closeDocument() (this=0x8468b10) at umldoc.cpp:284
     */
    if (associations())
        uDebug() << "UMLCanvasObject destructor: FIXME: there are still associations()";
}

/**
 * Return the subset of m_List that matches the given type.
 *
 * @param assocType   The Association_Type to match.
 * @return   The list of associations that match assocType.
 */
UMLAssociationList UMLCanvasObject::getSpecificAssocs(Uml::Association_Type assocType)
{
    UMLAssociationList list;
    UMLObject *o = NULL;
    for (UMLObjectListIt oit(m_List); oit.hasNext(); ) {
        o = oit.next();
        if (o->getBaseType() != Uml::ot_Association)
            continue;
        UMLAssociation *a = static_cast<UMLAssociation*>(o);
        if (a->getAssocType() == assocType)
            list.append(a);
    }
    return list;
}

/**
 * Adds an association end to m_List.
 *
 * @param assoc  The association to add.
 *               @todo change param type to UMLRole
 */
bool UMLCanvasObject::addAssociationEnd(UMLAssociation* assoc)
{
    // add association only if not already present in list
    if(!hasAssociation(assoc))
    {
        m_List.append( assoc );

        // Don't emit signals during load from XMI
        UMLObject::emitModified();
        emit sigAssociationEndAdded(assoc);
        return true;
    }
    return false;
}

/**
 * Determine if this canvasobject has the given association.
 *
 * @param assoc   The association to check.
 */
bool UMLCanvasObject::hasAssociation(UMLAssociation* assoc)
{
    if(m_List.count(assoc) > 0)
        return true;
    return false;
}

/**
 * Remove an association end from the CanvasObject.
 *
 * @param assoc   The association to remove.
 *                @todo change param type to UMLRole
 */
int UMLCanvasObject::removeAssociationEnd(UMLAssociation * assoc)
{
    if(!hasAssociation(assoc) || !m_List.removeAll(assoc)) {
        uWarning() << "can not find given assoc in list";
        return -1;
    }
    UMLObject::emitModified();
    emit sigAssociationEndRemoved(assoc);
    return m_List.count();
}

/**
 * Remove all association ends from the CanvasObject.
 */
void UMLCanvasObject::removeAllAssociationEnds()
{
    for (int i = 0; i < m_List.count(); ) {
        UMLObject *o = m_List.at(i);
        if (o->getBaseType() != Uml::ot_Association) {
            ++i;
            continue;
        }
        UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
        //umldoc->slotRemoveUMLObject(assoc);
        UMLObject* objA = assoc->getObject(Uml::A);
        UMLObject* objB = assoc->getObject(Uml::B);
        UMLCanvasObject *roleAObj = dynamic_cast<UMLCanvasObject*>(objA);
        if (roleAObj) {
            roleAObj->removeAssociationEnd(assoc);
        } else if (objA)
            uDebug() << m_Name << ": objA " << objA->getName() << " is not a UMLCanvasObject";
        else
            uDebug() << m_Name << "): objA is NULL";
        UMLCanvasObject *roleBObj = dynamic_cast<UMLCanvasObject*>(objB);
        if (roleBObj) {
            roleBObj->removeAssociationEnd(assoc);
        } else if (objB)
            uDebug() << m_Name << "): objB " << objB->getName() << " is not a UMLCanvasObject";
        else
            uDebug() << m_Name << "): objB is NULL";
        m_List.removeAt(i);
    }
}

void UMLCanvasObject::removeAllChildObjects()
{
    removeAllAssociationEnds();
    while ( !m_List.isEmpty() ) {
        delete m_List.takeFirst();
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
 * @return  Unique name string for the Object_Type given.
 */
QString UMLCanvasObject::uniqChildName( const Uml::Object_Type type,
                                        const QString &prefix /* = QString() */ )
{
    QString currentName;
    currentName = prefix;
    if (currentName.isEmpty()) {
        switch (type) {
            case Uml::ot_Association:
                currentName = i18n("new_association");
                break;
            case Uml::ot_Attribute:
                currentName = i18n("new_attribute");
                break;
            case Uml::ot_Template:
                currentName = i18n("new_template");
                break;
            case Uml::ot_Operation:
                currentName = i18n("new_operation");
                break;
            case Uml::ot_EnumLiteral:
                currentName = i18n("new_literal");
                break;
            case Uml::ot_EntityAttribute:
                currentName = i18n("new_field");
                break;
            case Uml::ot_UniqueConstraint:
                currentName = i18n( "new_unique_constraint" );
                break;
            case Uml::ot_ForeignKeyConstraint:
                currentName = i18n( "new_fkey_constraint" );
                break;
            case Uml::ot_CheckConstraint:
                currentName = i18n( "new_check_constraint" );
                break;
            default:
                uWarning() << "uniqChildName() called for unknown child type " << type;
                return "ERROR_in_UMLCanvasObject_uniqChildName";
        }
    }

    QString name = currentName;
    for (int number = 1; findChildObject(name); ++number) {
        name = currentName + '_' + QString::number(number);
    }
    return name;
}

/**
 * Find a child object with the given name.
 *
 * @param n         The name of the object to find.
 * @param t         The type to find (optional.) If not given then
 *                  any object type will match.
 * @return  Pointer to the object found; NULL if none found.
 */
UMLObject * UMLCanvasObject::findChildObject(const QString &n, Uml::Object_Type t)
{
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    UMLObject *obj = NULL;
    for (UMLObjectListIt oit(m_List); oit.hasNext(); ) {
        obj = oit.next();
        if (t != Uml::ot_UMLObject && obj->getBaseType() != t)
            continue;
        if (caseSensitive) {
            if (obj->getName() == n)
                return obj;
        } else if (obj->getName().toLower() == n.toLower()) {
            return obj;
        }
    }
    return NULL;
}

/**
 * Find an association.
 *
 * @param id        The id of the object to find.
 * @param considerAncestors boolean switch to consider ancestors while searching
 * @return  Pointer to the object found (NULL if not found.)
 */
UMLObject* UMLCanvasObject::findChildObjectById(Uml::IDType id, bool considerAncestors)
{
    Q_UNUSED(considerAncestors);
    UMLObject *o = NULL;
    for (UMLObjectListIt oit(m_List); oit.hasNext(); ) {
        o = oit.next();
        if (o->getID() == id)
            return o;
    }
    return 0;
}

bool UMLCanvasObject::operator==(const UMLCanvasObject& rhs)
{
    if (this == &rhs) {
        return true;
    }
    if ( !UMLObject::operator==(rhs) ) {
        return false;
    }
    if ( m_List.count() != rhs.m_List.count() ) {
        return false;
    }
    if ( &m_List != &(rhs.m_List) ) {
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
    //target->m_List = m_List;
}

/**
 * Returns the number of associations for the CanvasObject.
 * This is the sum of the aggregations and compositions.
 *
 * @return  The number of associations for the Concept.
 */
int UMLCanvasObject::associations()
{
    int count = 0;
    UMLObject *obj = NULL;
    for (UMLObjectListIt oit(m_List); oit.hasNext(); ) {
        obj = oit.next();
        if (obj->getBaseType() == Uml::ot_Association)
            count++;
    }
    return count;
}

/**
 * Return the list of associations for the CanvasObject.
 *
 * @return   The list of associations for the CanvasObject.
 */
UMLAssociationList UMLCanvasObject::getAssociations()
{
    UMLAssociationList assocs;
    UMLObject *o = NULL;
    for (UMLObjectListIt oit(m_List); oit.hasNext() ; ) {
        o = oit.next();
        if (o->getBaseType() != Uml::ot_Association)
            continue;
        UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
        assocs.append(assoc);
    }
    return assocs;
}

/**
 * Return a list of the superclasses of this concept.
 * TODO: This overlaps with UMLClassifier::findSuperClassConcepts(),
 *       see if we can merge the two.
 *
 * @return  The list of superclasses for the concept.
 */
UMLClassifierList UMLCanvasObject::getSuperClasses()
{
    UMLClassifierList list;
    UMLAssociationList assocs = getAssociations();
    foreach (UMLAssociation* a , assocs ) {
        if ((a->getAssocType() != Uml::at_Generalization &&
             a->getAssocType() != Uml::at_Realization) ||
                a->getObjectId(Uml::A) != getID() )
            continue;
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::B));
        if (c)
            list.append(c);
        else
            uDebug() << m_Name << ": generalization's other end is not a "
                << "UMLClassifier (id= " << ID2STR(a->getObjectId(Uml::B)) << ")";
    }
    return list;
}

/**
 * Return a list of the classes that inherit from this concept.
 * TODO: This overlaps with UMLClassifier::findSubClassConcepts(),
 *       see if we can merge the two.
 *
 * @return  The list of classes inheriting from the concept.
 */
UMLClassifierList UMLCanvasObject::getSubClasses()
{
    UMLClassifierList list;
    UMLAssociationList assocs = getAssociations();
    foreach (UMLAssociation* a , assocs ) {
        if ((a->getAssocType() != Uml::at_Generalization &&
             a->getAssocType() != Uml::at_Realization) ||
                a->getObjectId(Uml::B) != getID() )
            continue;
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
        if (c)
            list.append(c);
        else
            uDebug() << "specialization's other end is not a UMLClassifier"
                << " (id=" << ID2STR(a->getObjectId(Uml::A)) << ")";
    }
    return list;
}

/**
 * Shorthand for getSpecificAssocs(Uml::at_Realization)
 *
 * @return  The list of realizations for the Concept.
 */
UMLAssociationList UMLCanvasObject::getRealizations()
{
    return getSpecificAssocs(Uml::at_Realization);
}

/**
 * Shorthand for getSpecificAssocs(Uml::at_Aggregation)
 *
 * @return  The list of aggregations for the Concept.
 */
UMLAssociationList UMLCanvasObject::getAggregations()
{
    return getSpecificAssocs(Uml::at_Aggregation);
}

/**
 * Shorthand for getSpecificAssocs(Uml::at_Composition)
 *
 * @return  The list of compositions for the Concept.
 */
UMLAssociationList UMLCanvasObject::getCompositions()
{
    return getSpecificAssocs(Uml::at_Composition);
}

/**
 * Shorthand for getSpecificAssocs(Uml::at_Relationship)
 *
 * @return  The list of relationships for the entity.
 */
UMLAssociationList UMLCanvasObject::getRelationships()
{
    return getSpecificAssocs(Uml::at_Relationship);
}

/**
 * Reimplementation of UMLObject method.
 */
bool UMLCanvasObject::resolveRef()
{
    bool overallSuccess = UMLObject::resolveRef();
    for (UMLObjectListIt ait(m_List); ait.hasNext(); ) {
        UMLObject *obj = ait.next();
        if (! obj->resolveRef()) {
            m_List.removeAll(obj);
            overallSuccess = false;
        }
    }
    return overallSuccess;
}

#include "umlcanvasobject.moc"


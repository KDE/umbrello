/***************************************************************************
 *                                                                         *
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

// qt/kde includes
#include <kdebug.h>
#include <klocale.h>

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

bool UMLCanvasObject::hasAssociation(UMLAssociation* assoc)
{
    if(m_List.count(assoc) > 0)
        return true;
    return false;
}

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

QString UMLCanvasObject::uniqChildName( const Uml::Object_Type type,
                                        const QString &prefix /* = QString() */ )
{
    QString currentName = QString();
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

void UMLCanvasObject::copyInto(UMLObject *lhs) const
{
    UMLObject::copyInto(lhs);

    // TODO Associations are not copied at the moment. This because
    // the duplicate function (on umlwidgets) do not copy the associations.
    //
    //target->m_List = m_List;
}

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

UMLAssociationList UMLCanvasObject::getRealizations()
{
    return getSpecificAssocs(Uml::at_Realization);
}

UMLAssociationList UMLCanvasObject::getAggregations()
{
    return getSpecificAssocs(Uml::at_Aggregation);
}

UMLAssociationList UMLCanvasObject::getCompositions()
{
    return getSpecificAssocs(Uml::at_Composition);
}

UMLAssociationList UMLCanvasObject::getRelationships()
{
    return getSpecificAssocs(Uml::at_Relationship);
}

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


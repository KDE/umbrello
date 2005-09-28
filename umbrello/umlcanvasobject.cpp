/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "uml.h"
#include "umldoc.h"
#include "umlcanvasobject.h"
#include "classifier.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"
#include "template.h"
#include "stereotype.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLCanvasObject::UMLCanvasObject(const QString & name, Uml::IDType id)
        : UMLObject(name, id)
{
    init();
}

UMLCanvasObject::~UMLCanvasObject() {
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
        kdDebug() << "UMLCanvasObject destructor: FIXME: there are still associations()" << endl;
}

UMLAssociationList UMLCanvasObject::getSpecificAssocs(Uml::Association_Type assocType) {
    UMLAssociationList list;
    UMLObject *o;
    for (UMLObjectListIt oit(m_List); (o = oit.current()) != NULL; ++oit) {
        if (o->getBaseType() != Uml::ot_Association)
            continue;
        UMLAssociation *a = static_cast<UMLAssociation*>(o);
        if (a->getAssocType() == assocType)
            list.append(a);
    }
    return list;
}

bool UMLCanvasObject::addAssociation(UMLAssociation* assoc) {
    // add association only if not already present in list
    if(!hasAssociation(assoc))
    {
        m_List.append( assoc );
        emit modified();
        emit sigAssociationAdded(assoc);
        return true;
    }
    return false;
}

bool UMLCanvasObject::hasAssociation(UMLAssociation* assoc) {
    if(m_List.containsRef(assoc) > 0)
        return true;
    return false;
}

int UMLCanvasObject::removeAssociation(UMLAssociation * assoc) {
    if(!hasAssociation(assoc) || !m_List.remove(assoc)) {
        kdWarning() << "can't find assoc given in list" << endl;
        return -1;
    }
    emit modified();
    emit sigAssociationRemoved(assoc);
    return m_List.count();
}

void UMLCanvasObject::removeAllAssociations() {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    UMLObject *o;
    for (UMLObjectListIt oit(m_List); (o = oit.current()) != NULL; ++oit) {
        if (o->getBaseType() != Uml::ot_Association)
            continue;
        UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
        umldoc->slotRemoveUMLObject(assoc);
        UMLObject* objA = assoc->getObject(Uml::A);
        UMLObject* objB = assoc->getObject(Uml::B);
        UMLCanvasObject *roleAObj = dynamic_cast<UMLCanvasObject*>(objA);
        if (roleAObj)
            roleAObj->removeAssociation(assoc);
        UMLCanvasObject *roleBObj = dynamic_cast<UMLCanvasObject*>(objB);
        if (roleBObj)
            roleBObj->removeAssociation(assoc);
        //delete assoc;  cannot do this! AssociationWidgets may exist!
    }
}

QString UMLCanvasObject::uniqChildName( const Uml::Object_Type type,
                                        bool seekStereo /* = false */ ) {
    QString currentName;
    if (seekStereo) {
        currentName = i18n("new_stereotype");
    } else if (type == Uml::ot_Association) {
        currentName = i18n("new_association");
    } else if (type == Uml::ot_Attribute) {
        currentName = i18n("new_attribute");
    } else if (type == Uml::ot_Template) {
        currentName = i18n("new_template");
    } else if (type == Uml::ot_Operation) {
        currentName = i18n("new_operation");
    } else if (type == Uml::ot_EnumLiteral) {
        currentName = i18n("new_literal");
    } else if (type == Uml::ot_EntityAttribute) {
        currentName = i18n("new_field");
    } else {
        kdWarning() << "uniqChildName() called for unknown child type " << type << endl;
        return "ERROR_in_UMLCanvasObject_uniqChildName";
    }

    QString name = currentName;
    for (int number = 1; findChildObject(name); ++number) {
        name = currentName + "_" + QString::number(number);
    }
    return name;
}

UMLObject * UMLCanvasObject::findChildObject(const QString &n, Uml::Object_Type t) {
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    UMLObject *obj;
    for (UMLObjectListIt oit(m_List); (obj = oit.current()) != NULL; ++oit) {
        if (t != Uml::ot_UMLObject && obj->getBaseType() != t)
            continue;
        if (caseSensitive) {
            if (obj->getName() == n)
                return obj;
        } else if (obj->getName().lower() == n.lower()) {
            return obj;
        }
    }
    return NULL;
}

UMLObject* UMLCanvasObject::findChildObjectById(Uml::IDType id, bool /* considerAncestors */) {
    UMLObject *o;
    for (UMLObjectListIt oit(m_List); (o = oit.current()) != NULL; ++oit) {
        if (o->getID() == id)
            return o;
    }
    return 0;
}

void UMLCanvasObject::init() {
    m_List.setAutoDelete(false);
}

bool UMLCanvasObject::operator==(UMLCanvasObject& rhs) {
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

void UMLCanvasObject::copyInto(UMLCanvasObject *rhs) const
{
    UMLObject::copyInto(rhs);

    // TODO Associations are not copied at the moment. This because
    // the duplicate function (on umlwidgets) do not copy the associations.
    //
    //rhs->m_List = m_List;
}

int UMLCanvasObject::associations() {
    int count = 0;
    UMLObject *obj;
    for (UMLObjectListIt oit(m_List); (obj = oit.current()) != NULL; ++oit) {
        if (obj->getBaseType() == Uml::ot_Association)
            count++;
    }
    return count;
}

UMLAssociationList UMLCanvasObject::getAssociations() {
    UMLAssociationList assocs;
    UMLObject *o;
    for (UMLObjectListIt oit(m_List); (o = oit.current()) != NULL; ++oit) {
        if (o->getBaseType() != Uml::ot_Association)
            continue;
        UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
        assocs.append(assoc);
    }
    return assocs;
}

UMLClassifierList UMLCanvasObject::getSuperClasses() {
    UMLClassifierList list;
    UMLAssociationList assocs = getAssociations();
    for (UMLAssociation* a = assocs.first(); a; a = assocs.next()) {
        if ( a->getAssocType() != Uml::at_Generalization ||
                a->getObjectId(Uml::A) != getID() )
            continue;
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::B));
        if (c)
            list.append(c);
        else
            kdDebug() << "UMLCanvasObject::getSuperClasses(" << m_Name
            << "): generalization's other end is not a "
            << "UMLClassifier (id= " << ID2STR(a->getObjectId(Uml::B)) << ")"
            << endl;
    }
    return list;
}

UMLClassifierList UMLCanvasObject::getSubClasses() {
    UMLClassifierList list;
    UMLAssociationList assocs = getAssociations();
    for (UMLAssociation* a = assocs.first(); a; a = assocs.next()) {
        if ( a->getAssocType() != Uml::at_Generalization ||
                a->getObjectId(Uml::B) != getID() )
            continue;
        UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
        if (c)
            list.append(c);
        else
            kdDebug() << "UMLCanvasObject::getSubClasses: specialization's"
            << " other end is not a UMLClassifier"
            << " (id=" << ID2STR(a->getObjectId(Uml::A)) << ")" << endl;
    }
    return list;
}

UMLAssociationList UMLCanvasObject::getRealizations() {
    return getSpecificAssocs(Uml::at_Realization);
}

UMLAssociationList UMLCanvasObject::getAggregations() {
    return getSpecificAssocs(Uml::at_Aggregation);
}

UMLAssociationList UMLCanvasObject::getCompositions() {
    return getSpecificAssocs(Uml::at_Composition);
}

UMLAssociationList UMLCanvasObject::getRelationships() {
    return getSpecificAssocs(Uml::at_Relationship);
}
#include "umlcanvasobject.moc"

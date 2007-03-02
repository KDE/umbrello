/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header file
#include "package.h"

// system includes
#include <kdebug.h>
#include <klocale.h>

// local includes
#include "uml.h"
#include "umldoc.h"
#include "classifier.h"
#include "association.h"
#include "object_factory.h"
#include "model_utils.h"
#include "umllistview.h"
#include "umllistviewitem.h"

using namespace Uml;

UMLPackage::UMLPackage(const QString & name, Uml::IDType id)
        : UMLCanvasObject(name, id) {
    init();
}

UMLPackage::~UMLPackage() {
}

void UMLPackage::init() {
    m_BaseType = ot_Package;
}

void UMLPackage::copyInto(UMLPackage *rhs) const
{
    UMLCanvasObject::copyInto(rhs);

    m_objects.copyInto(&(rhs->m_objects));
}

UMLObject* UMLPackage::clone() const
{
    UMLPackage *clone = new UMLPackage();
    copyInto(clone);

    return clone;
}

void UMLPackage::addAssocToConcepts(UMLAssociation* a) {
    if (! UMLAssociation::assocTypeHasUMLRepresentation(a->getAssocType()) )
        return;
    Uml::IDType AId = a->getObjectId(Uml::A);
    Uml::IDType BId = a->getObjectId(Uml::B);
    UMLObject *o;
    for (UMLObjectListIt it(m_objects); (o = it.current()) != NULL; ++it) {
        UMLCanvasObject *c = dynamic_cast<UMLCanvasObject*>(o);
        if (c == NULL)
            continue;
        if (AId == c->getID() || (BId == c->getID())) {
            if (c->hasAssociation(a))
                kDebug() << "UMLPackage::addAssocToConcepts: " << c->getName()
                    << " already has association id=" << ID2STR(a->getID())
                    << endl;
            else
               c->addAssociationEnd(a);
        }
        UMLPackage *pkg = dynamic_cast<UMLPackage*>(c);
        if (pkg)
            pkg->addAssocToConcepts(a);
    }
}

void UMLPackage::removeAssocFromConcepts(UMLAssociation *assoc)
{
    UMLObject *o;
    for (UMLObjectListIt it(m_objects); (o = it.current()) != NULL; ++it) {
        UMLCanvasObject *c = dynamic_cast<UMLCanvasObject*>(o);
        if (c == NULL)
            continue;
        if (c->hasAssociation(assoc))
            c->removeAssociationEnd(assoc);
        UMLPackage *pkg = dynamic_cast<UMLPackage*>(c);
        if (pkg)
            pkg->removeAssocFromConcepts(assoc);
    }
}

bool UMLPackage::addObject(UMLObject *pObject) {
    if (pObject == NULL) {
        kError() << "UMLPackage::addObject is called with a NULL object"
            << endl;
        return false;
    }
    if (m_objects.find(pObject) != -1) {
        kDebug() << "UMLPackage::addObject: " << pObject->getName()
                  << " is already there" << endl;
        return false;
    }
    if (pObject->getBaseType() == Uml::ot_Association) {
        UMLAssociation *assoc = static_cast<UMLAssociation*>(pObject);
        // Adding the UMLAssociation at the participating concepts is done
        // again later (in UMLAssociation::resolveRef()) if they are not yet
        // known right here.
        if (assoc->getObject(Uml::A) && assoc->getObject(Uml::B)) {
            UMLPackage *pkg = pObject->getUMLPackage();
            if (pkg != this) {
               kError() << "UMLPackage " << m_Name << " addObject: "
                   << "assoc's UMLPackage is " << pkg->getName() << endl;
            }
            addAssocToConcepts(assoc);
        }
    }
    m_objects.append( pObject );
    return true;
}

void UMLPackage::removeObject(UMLObject *pObject) {
    if (pObject->getBaseType() == Uml::ot_Association) {
        UMLObject *o = const_cast<UMLObject*>(pObject);
        UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
        removeAssocFromConcepts(assoc);
    }
    if (m_objects.findRef(pObject) == -1)
        kDebug() << m_Name << " removeObject: object with id="
            << ID2STR(pObject->getID()) << "not found." << endl;
    else
        m_objects.remove(pObject);
}

void UMLPackage::removeAllObjects() {
    UMLCanvasObject::removeAllChildObjects();
    UMLObject *o;
    while ((o = m_objects.first()) != NULL) {
        UMLPackage *pkg = dynamic_cast<UMLPackage*>(o);
        if (pkg)
            pkg->removeAllObjects();
        removeObject(o);
        //delete o;
        // CHECK: Direct usage of the destructor crashes on associations.
        o->deleteLater();
    }
}

UMLObjectList UMLPackage::containedObjects() {
    return m_objects;
}

UMLObject * UMLPackage::findObject(const QString &name) {
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if (caseSensitive) {
            if (obj->getName() == name)
                return obj;
        } else if (obj->getName().lower() == name.lower()) {
            return obj;
        }
    }
    return NULL;
}

UMLObject * UMLPackage::findObjectById(Uml::IDType id) {
    return Model_Utils::findObjectInList(id, m_objects);
}

void UMLPackage::appendClassifiers(UMLClassifierList& classifiers,
                                   bool includeNested /* = true */) {
    for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
        UMLObject *o = oit.current();
        Object_Type ot = o->getBaseType();
        if (ot == ot_Class || ot == ot_Interface ||
                ot == ot_Datatype || ot == ot_Enum || ot == ot_Entity) {
            classifiers.append((UMLClassifier *)o);
        } else if (includeNested && (ot == ot_Package || ot == ot_Folder)) {
            UMLPackage *inner = static_cast<UMLPackage *>(o);
            inner->appendClassifiers(classifiers);
        }
    }
}

void UMLPackage::appendClasses(UMLClassifierList& classes,
                               bool includeNested /* = true */) {
    for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
        UMLObject *o = oit.current();
        Object_Type ot = o->getBaseType();
        if (ot == ot_Class) {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            classes.append(c);
        } else if (includeNested && (ot == ot_Package || ot == ot_Folder)) {
            UMLPackage *inner = static_cast<UMLPackage *>(o);
            inner->appendClasses(classes);
        }
    }
}

void UMLPackage::appendClassesAndInterfaces(UMLClassifierList& classifiers,
        bool includeNested /* = true */) {
    for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
        UMLObject *o = oit.current();
        Object_Type ot = o->getBaseType();
        if (ot == ot_Class || ot == ot_Interface) {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            classifiers.append(c);
        } else if (includeNested && (ot == ot_Package || ot == ot_Folder)) {
            UMLPackage *inner = static_cast<UMLPackage *>(o);
            inner->appendClassesAndInterfaces(classifiers);
        }
    }
}

void UMLPackage::appendInterfaces( UMLClassifierList& interfaces,
                                   bool includeNested /* = true */) {
    for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
        UMLObject *o = oit.current();
        Object_Type ot = o->getBaseType();
        if (ot == ot_Interface) {
            UMLClassifier *c = static_cast<UMLClassifier*>(o);
            interfaces.append(c);
        } else if (includeNested && (ot == ot_Package || ot == ot_Folder)) {
            UMLPackage *inner = static_cast<UMLPackage *>(o);
            inner->appendInterfaces(interfaces);
        }
    }
}

bool UMLPackage::resolveRef() {
    bool overallSuccess = UMLCanvasObject::resolveRef();
    for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if (! obj->resolveRef()) {
            Uml::Object_Type ot = obj->getBaseType();
            if (ot != Uml::ot_Package && ot != Uml::ot_Folder)
                m_objects.remove(obj);
            overallSuccess = false;
        }
    }
    return overallSuccess;
}

void UMLPackage::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement packageElement = UMLObject::save("UML:Package", qDoc);
    QDomElement ownedElement = qDoc.createElement("UML:Namespace.ownedElement");
    UMLObject *obj;
    // save classifiers etc.
    for (UMLObjectListIt oit(m_objects); (obj = oit.current()) != NULL; ++oit)
        obj->saveToXMI (qDoc, ownedElement);
    // save associations
    for (UMLObjectListIt ait(m_List); (obj = ait.current()) != NULL; ++ait)
        obj->saveToXMI (qDoc, ownedElement);

    packageElement.appendChild(ownedElement);
    qElement.appendChild(packageElement);
}

bool UMLPackage::load(QDomElement& element) {
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (Model_Utils::isCommonXMIAttribute(type))
            continue;
        if (tagEq(type, "Namespace.ownedElement") ||
                tagEq(type, "Namespace.contents")) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load(tempElement))
                return false;
            continue;
        }
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type);
        if( !pObject ) {
            kWarning() << "UMLPackage::load: "
                << "Unknown type of umlobject to create: "
                << type << endl;
            continue;
        }
        pObject->setUMLPackage(this);
        if (!pObject->loadFromXMI(tempElement)) {
            removeObject(pObject);
            delete pObject;
        }
    }
    return true;
}

#include "package.moc"

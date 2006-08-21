/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                 *
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

bool UMLPackage::addObject(const UMLObject *pObject) {
    if (pObject == NULL) {
        kdError() << "UMLPackage::addObject is called with a NULL object"
            << endl;
        return false;
    }
    if (pObject->getBaseType() == Uml::ot_Association) {
        UMLObject *o = const_cast<UMLObject*>(pObject);
        UMLAssociation *assoc = static_cast<UMLAssociation*>(o);
        return UMLCanvasObject::addAssociation(assoc);
    }
    if (m_objects.find(pObject) != -1) {
        kdDebug() << "UMLPackage::addObject: " << pObject->getName()
                  << " is already there" << endl;
        return false;
    }
    m_objects.append( pObject );
    return true;
}

void UMLPackage::removeObject(const UMLObject *pObject) {
    m_objects.remove( pObject );
}

void UMLPackage::removeAllObjects() {
    UMLCanvasObject::removeAllAssociations();
    UMLObject *o;
    for (UMLObjectListIt oit(m_objects); (o = oit.current()) != NULL; ++oit) {
        UMLPackage *pkg = dynamic_cast<UMLPackage*>(o);
        if (pkg)
            pkg->removeAllObjects();
        //delete o;
    }
    m_objects.clear();
}

UMLObjectList UMLPackage::containedObjects(bool includeAssociations) {
    UMLObjectList list = m_objects;
    if (includeAssociations) {
        UMLObject *o;
        for (UMLObjectListIt oit(m_List); (o = oit.current()) != NULL; ++oit) {
            list.append(o);
        }
    }
    return list;
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
        if (! obj->resolveRef())
            overallSuccess = false;
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
            kdWarning() << "UMLPackage::load: "
                << "Unknown type of umlobject to create: "
                << type << endl;
            continue;
        }
        pObject->setUMLPackage(this);
        if (pObject->loadFromXMI(tempElement)) {
            addObject(pObject);
            /*
            UMLAssociation *assoc = dynamic_cast<UMLAssociation*>(pObject);
            if (assoc) {
                // Adding the UMLAssociation at the participating concepts is done
                // again later (in UMLAssociation::resolveRef()) if they are not yet
                // known right here.
                if (assoc->getObject(Uml::A) && assoc->getObject(Uml::B))
                    umldoc->addAssocToConcepts(Assoc);
            }
             */
        } else {
            delete pObject;
        }
    }
    return true;
}

#include "package.moc"

/*
 *  copyright (C) 2003-2005
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
#include "object_factory.h"
#include "model_utils.h"

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
    if (m_objects.find(pObject) != -1) {
        kDebug() << "UMLPackage::addObject: " << pObject->getName()
                  << " is already there" << endl;
        return false;
    }
    m_objects.append( pObject );
    return true;
}

void UMLPackage::removeObject(const UMLObject *pObject) {
    m_objects.remove( pObject );
}

UMLObjectList& UMLPackage::containedObjects() {
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
                ot == ot_Datatype || ot == ot_Enum) {
            classifiers.append((UMLClassifier *)o);
        } else if (includeNested && ot == ot_Package) {
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
        } else if (includeNested && ot == ot_Package) {
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
        } else if (includeNested && ot == ot_Package) {
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
        } else if (includeNested && ot == ot_Package) {
            UMLPackage *inner = static_cast<UMLPackage *>(o);
            inner->appendInterfaces(interfaces);
        }
    }
}

bool UMLPackage::resolveRef() {
    // UMLObject::resolveRef() is not required by ot_Package itself
    // but might be required by some inheriting class.
    bool overallSuccess = UMLObject::resolveRef();
    for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
        UMLObject *obj = oit.current();
        if (! obj->resolveRef())
            overallSuccess = false;
    }
    return overallSuccess;
}

void UMLPackage::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement packageElement = UMLObject::save("UML:Package", qDoc);
    QDomElement ownedElement = qDoc.createElement( "UML:Namespace.ownedElement" );

#ifndef XMI_FLAT_PACKAGES
    // Save datatypes first.
    // This will cease to be necessary once deferred type resolution is up.
    for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
        if (obj->getBaseType() == Uml::ot_Datatype)
            obj->saveToXMI (qDoc, ownedElement);
    for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
        if (obj->getBaseType() != Uml::ot_Datatype)
            obj->saveToXMI (qDoc, ownedElement);
#endif
    packageElement.appendChild( ownedElement );
    qElement.appendChild(packageElement);
}

bool UMLPackage::load(QDomElement& element) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
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
        if (pObject->loadFromXMI(tempElement)) {
            addObject(pObject);
            if (tagEq(type, "Generalization"))
                umldoc->addAssocToConcepts((UMLAssociation *) pObject);
        } else {
            delete pObject;
        }
    }
    return true;
}

#include "package.moc"

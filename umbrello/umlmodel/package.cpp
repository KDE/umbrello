/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header file
#include "package.h"

// local includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "classifier.h"
#include "association.h"
#include "entity.h"
#include "object_factory.h"
#include "model_utils.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QInputDialog>

using namespace Uml;

/**
 * Sets up a Package.
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLPackage::UMLPackage(const QString & name, Uml::ID::Type id)
        : UMLCanvasObject(name, id)
{
    m_BaseType = ot_Package;
}

/**
 * Destructor.
 */
UMLPackage::~UMLPackage()
{
}

/**
 * Copy the internal presentation of this object into the new object.
 */
void UMLPackage::copyInto(UMLObject *lhs) const
{
    UMLPackage *target = lhs->asUMLPackage();

    UMLCanvasObject::copyInto(target);

    m_objects.copyInto(&(target->m_objects));
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLPackage::clone() const
{
    UMLPackage *clone = new UMLPackage();
    copyInto(clone);

    return clone;
}

/**
 * Adds an existing association to the matching concept in the list of concepts.
 * The selection of the matching concept depends on the association type:
 * For generalizations, the assoc is added to the concept that matches role A.
 * For aggregations and compositions, the assoc is added to the concept
 * that matches role B.
 * @param assoc   the association to add
 */
void UMLPackage::addAssocToConcepts(UMLAssociation* assoc)
{
    if (! AssociationType::hasUMLRepresentation(assoc->getAssocType()))
        return;
    Uml::ID::Type AId = assoc->getObjectId(Uml::RoleType::A);
    Uml::ID::Type BId = assoc->getObjectId(Uml::RoleType::B);
    UMLObject *o = 0;
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        o = oit.next();
        UMLCanvasObject *c = o->asUMLCanvasObject();
        if (c == 0)
            continue;
        if (AId == c->id() || (BId == c->id())) {
            if (c->hasAssociation(assoc))
                uDebug() << c->name() << " already has association id=" << Uml::ID::toString(assoc->id());
            else
               c->addAssociationEnd(assoc);
        }
        UMLPackage *pkg = c->asUMLPackage();
        if (pkg)
            pkg->addAssocToConcepts(assoc);
    }
}

/**
 * Remove the association from the participating concepts.
 * @param assoc   the association to remove
 */
void UMLPackage::removeAssocFromConcepts(UMLAssociation *assoc)
{
    UMLObject *o = 0;
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        o = oit.next();
        UMLCanvasObject *c = o->asUMLCanvasObject();
        if (c) {
            if (c->hasAssociation(assoc))
                c->removeAssociationEnd(assoc);
            UMLPackage *pkg = c->asUMLPackage();
            if (pkg)
                pkg->removeAssocFromConcepts(assoc);
        }
    }
}

/**
 * Adds an object in this package.
 *
 * @param pObject   Pointer to the UMLObject to add.
 * @return    True if the object was actually added.
 */
bool UMLPackage::addObject(UMLObject *pObject)
{
    if (pObject == 0) {
        uError() << "is called with a NULL object";
        return false;
    }
    if (pObject == this) {
        uError() << "adding myself as child is not allowed";
        return false;
    }

    if (m_objects.indexOf(pObject) != -1) {
        uDebug() << pObject->name() << " is already there";
        return false;
    }
    if (pObject->baseType() == UMLObject::ot_Association) {
        UMLAssociation *assoc = pObject->asUMLAssociation();
        // Adding the UMLAssociation at the participating concepts is done
        // again later (in UMLAssociation::resolveRef()) if they are not yet
        // known right here.
        if (assoc->getObject(Uml::RoleType::A) && assoc->getObject(Uml::RoleType::B)) {
            UMLPackage *pkg = pObject->umlPackage();
            if (pkg != this) {
               uError() << "UMLPackage " << name() << " addObject: "
                        << "assoc's UMLPackage is " << pkg->name();
            }
            addAssocToConcepts(assoc);
        }
    }
    else {
      QString name = pObject->name();
      QString oldName = name;
      while (findObject(name) != 0 && pObject->baseType() != UMLObject::ot_Instance) {
         QString prevName = name;
         name = Model_Utils::uniqObjectName(pObject->baseType(), this);
         bool ok = Dialog_Utils::askName(i18nc("object name", "Name"),
                                         i18n("An object with the name %1\nalready exists in the package %2.\nPlease enter a new name:", prevName, this->name()),
                                         name);
         if (!ok) {
            name = oldName;
            continue;
         }
         if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."),
                               i18n("Invalid Name"));
            continue;
        }
      }
      if (oldName != name) {
        pObject->setName(name);
      }
    }
    m_objects.append(pObject);
    return true;
}

/**
 * Removes an object from this package.
 * Does not physically delete the object.
 *
 * @param pObject   Pointer to the UMLObject to be removed.
 */
void UMLPackage::removeObject(UMLObject *pObject)
{
    if (pObject->baseType() == UMLObject::ot_Association) {
        UMLObject *o = const_cast<UMLObject*>(pObject);
        UMLAssociation *assoc = o->asUMLAssociation();
        removeAssocFromConcepts(assoc);
    }
    if (m_objects.indexOf(pObject) == -1)
        uDebug() << name() << " removeObject: object with id="
                 << Uml::ID::toString(pObject->id()) << "not found.";
    else
        m_objects.removeAll(pObject);
}

/**
 * Removes all objects from this package.
 * Inner containers (e.g. nested packages) are removed recursively.
 */
void UMLPackage::removeAllObjects()
{
    UMLCanvasObject::removeAllChildObjects();
    UMLObject *o = 0;

    while (!m_objects.isEmpty() && (o = m_objects.first()) != 0)  {
        UMLPackage *pkg = o->asUMLPackage();
        if (pkg)
            pkg->removeAllObjects();
        removeObject(o);
        delete o;
    }
}

/**
 * Returns the list of objects contained in this package.
 */
UMLObjectList UMLPackage::containedObjects()
{
    return m_objects;
}

/**
 * Find the object of the given name in the list of contained objects.
 *
 * @param name   The name to seek.
 * @return  Pointer to the UMLObject found or NULL if not found.
 */
UMLObject * UMLPackage::findObject(const QString &name)
{
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        UMLObject *obj = oit.next();
        if (!obj)
            continue;
        if (caseSensitive) {
            if (obj->name() == name)
                return obj;
        } else if (obj->name().toLower() == name.toLower()) {
            return obj;
        }
    }
    return 0;
}

/**
 * Find the object of the given ID in the list of contained objects.
 *
 * @param id   The ID to seek.
 * @return  Pointer to the UMLObject found or NULL if not found.
 */
UMLObject * UMLPackage::findObjectById(Uml::ID::Type id)
{
    return Model_Utils::findObjectInList(id, m_objects);
}

/**
 * Append all packages from this package (and those from nested packages)
 * to the given UMLPackageList.
 *
 * @param packages        The list to append to
 * @param includeNested   Whether to include the packages from nested packages
 *                        (default:true)
 */
void UMLPackage::appendPackages(UMLPackageList& packages, bool includeNested)
{
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        UMLObject *o = oit.next();
        uIgnoreZeroPointer(o);
        ObjectType ot = o->baseType();
        if (ot == ot_Package || ot == ot_Folder) {
            packages.append(o->asUMLPackage());
            if (includeNested) {
               UMLPackage *inner = o->asUMLPackage();
               inner->appendPackages(packages);
            }
         }
    }
}

/**
 * Append all classifiers from this package (and those from
 * nested packages) to the given UMLClassifierList.
 *
 * @param classifiers     The list to append to.
 * @param includeNested   Whether to include the classifiers from
 *                        nested packages (default: true.)
 */
void UMLPackage::appendClassifiers(UMLClassifierList& classifiers,
                                   bool includeNested /* = true */)
{
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        UMLObject *o = oit.next();
        uIgnoreZeroPointer(o);
        ObjectType ot = o->baseType();
        if (ot == ot_Class || ot == ot_Interface ||
                ot == ot_Datatype || ot == ot_Enum || ot == ot_Entity) {
            classifiers.append((UMLClassifier *)o);
        } else if (includeNested && (ot == ot_Package || ot == ot_Folder)) {
            UMLPackage *inner = o->asUMLPackage ();
            inner->appendClassifiers(classifiers);
        }
    }
}

/**
 * Append all entities from this package (and those
 * from nested packages) to the given UMLEntityList.
 *
 * @param entities        The list to append to.
 * @param includeNested   Whether to include the entities from
 *                        nested packages (default: true.)
 */
void UMLPackage::appendEntities(UMLEntityList& entities,
                                 bool includeNested /* = true */)
{
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        UMLObject *o = oit.next();
        uIgnoreZeroPointer(o);
        ObjectType ot = o->baseType();
        if (ot == ot_Entity) {
            UMLEntity *c = o->asUMLEntity();
            entities.append(c);
        } else if (includeNested && (ot == ot_Package || ot == ot_Folder)) {
            UMLPackage *inner = o->asUMLPackage ();
            inner->appendEntities(entities);
        }
    }
}

/**
 * Append all classes and interfaces from this package (and those
 * from nested packages) to the given UMLClassifierList.
 *
 * @param classifiers     The list to append to.
 * @param includeNested   Whether to include the classifiers from
 *                        nested packages (default: true.)
 */
void UMLPackage::appendClassesAndInterfaces(UMLClassifierList& classifiers,
        bool includeNested /* = true */)
{
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        UMLObject *o = oit.next();
        uIgnoreZeroPointer(o);
        ObjectType ot = o->baseType();
        if (ot == ot_Class || ot == ot_Interface) {
            UMLClassifier *c = o->asUMLClassifier();
            classifiers.append(c);
        } else if (includeNested && (ot == ot_Package || ot == ot_Folder)) {
            UMLPackage *inner = o->asUMLPackage ();
            inner->appendClassesAndInterfaces(classifiers);
        }
    }
}

/**
 * Resolve types. Required when dealing with foreign XMI files.
 * Needs to be called after all UML objects are loaded from file.
 * Overrides the method from UMLObject.
 * Calls resolveRef() on each contained object.
 *
 * @return  True for overall success.
 */
bool UMLPackage::resolveRef()
{
    bool overallSuccess = UMLCanvasObject::resolveRef();
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        UMLObject *obj = oit.next();
        uIgnoreZeroPointer(obj);
        if (! obj->resolveRef()) {
            UMLObject::ObjectType ot = obj->baseType();
            if (ot != UMLObject::ot_Package && ot != UMLObject::ot_Folder)
                m_objects.removeAll(obj);
            overallSuccess = false;
        }
    }
    return overallSuccess;
}

/**
 * Creates the <UML:Package> XMI element.
 */
void UMLPackage::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement packageElement = UMLObject::save(QLatin1String("UML:Package"), qDoc);
    QDomElement ownedElement = qDoc.createElement(QLatin1String("UML:Namespace.ownedElement"));
    UMLObject *obj = 0;
    // save classifiers etc.
    for (UMLObjectListIt oit(m_objects); oit.hasNext();) {
        obj = oit.next();
        uIgnoreZeroPointer(obj);
        obj->saveToXMI (qDoc, ownedElement);
    }
    // save associations
    for (UMLObjectListIt ait(m_List); ait.hasNext();) {
        obj = ait.next();
        obj->saveToXMI (qDoc, ownedElement);
    }

    packageElement.appendChild(ownedElement);
    qElement.appendChild(packageElement);
}

/**
 * Loads the <UML:Package> XMI element.
 * Auxiliary to UMLObject::loadFromXMI.
 */
bool UMLPackage::load(QDomElement& element)
{
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (Model_Utils::isCommonXMIAttribute(type))
            continue;
        if (UMLDoc::tagEq(type, QLatin1String("Namespace.ownedElement")) ||
                UMLDoc::tagEq(type, QLatin1String("Element.ownedElement")) ||  // Embarcadero's Describe
                UMLDoc::tagEq(type, QLatin1String("Namespace.contents"))) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load(tempElement))
                return false;
            continue;
        } else if (UMLDoc::tagEq(type, QLatin1String("packagedElement")) ||
                   UMLDoc::tagEq(type, QLatin1String("ownedElement"))) {
            type = tempElement.attribute(QLatin1String("xmi:type"));
        }
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type);
        if(!pObject) {
            uWarning() << "Unknown type of umlobject to create: " << type;
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


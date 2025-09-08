/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header file
#include "umlpackage.h"

// local includes
#include "debug_utils.h"
#include "dialog_utils.h"
#include "umlapp.h"
#include "umldoc.h"
#include "umlclassifier.h"
#include "umlassociation.h"
#include "umldatatype.h"
#include "umlentity.h"
#include "umlfolder.h"
#include "object_factory.h"
#include "optionstate.h"
#include "model_utils.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes

using namespace Uml;

DEBUG_REGISTER(UMLPackage)

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
 * Adds an existing association to the matching classifier in the list of concepts.
 * The selection of the matching classifier depends on the association type:
 * For generalizations, the assoc is added to the classifier that matches role A.
 * For aggregations and compositions, the assoc is added to the classifier
 * that matches role B.
 * @param assoc   the association to add
 */
void UMLPackage::addAssocToConcepts(UMLAssociation* assoc)
{
    if (! AssociationType::hasUMLRepresentation(assoc->getAssocType()))
        return;
    Uml::ID::Type AId = assoc->getObjectId(Uml::RoleType::A);
    Uml::ID::Type BId = assoc->getObjectId(Uml::RoleType::B);
    for(UMLObject *o : m_objects) {
        uIgnoreZeroPointer(o);
        UMLCanvasObject *c = o->asUMLCanvasObject();
        if (c == nullptr)
            continue;
        if (AId == c->id() || (BId == c->id())) {
            if (c->hasAssociation(assoc)) {
                logDebug2("UMLPackage::addAssocToConcepts: %1 already has association id=%2",
                          c->name(), Uml::ID::toString(assoc->id()));
            } else {
               c->addAssociationEnd(assoc);
            }
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
    for(UMLObject *o : m_objects) {
        uIgnoreZeroPointer(o);
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
 * @param pObject             Pointer to the UMLObject to add.
 * @param interactOnConflict  If pObject's name is already present in the package's
 *                            contained objects then
 *                            - if true then open a dialog asking the user for a
 *                              different name;
 *                            - if false then return false without inserting pObject.
 * @return    True if the object was actually added.
 */
bool UMLPackage::addObject(UMLObject *pObject, bool interactOnConflict /* = true */) {
    if (pObject == nullptr) {
        logError0("UMLPackage::addObject is called with a null object");
        return false;
    }
    if (pObject == this) {
        logError0("UMLPackage::addObject: adding self as child is not allowed");
        return false;
    }

    if (m_objects.indexOf(pObject) != -1) {
        logDebug1("UMLPackage::addObject %1: object is already there", pObject->name());
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
               logError2("UMLPackage %1 addObject: assoc's UMLPackage is %2", name(), pkg->name());
            }
            addAssocToConcepts(assoc);
        }
    }
    else if (interactOnConflict) {
      QString name = pObject->name();
      QString oldName = name;
      UMLObject *o;
      while ((o = findObject(name)) != nullptr && o->baseType() != UMLObject::ot_Association) {
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
            KMessageBox::error(nullptr, i18n("That is an invalid name."),
                               i18n("Invalid Name"));
            continue;
        }
      }
      if (oldName != name) {
        pObject->setName(name);
      }
    }
    else {
        QString nameToAdd = pObject->name();
        bool found = false;
        for(const UMLObject *obj : m_objects) {
            if (obj->name() == nameToAdd) {
                found = true;
                break;
            }
        }
        if (found) {
            logDebug2("UMLPackage %1 addObject: name %2 is already there", name(), nameToAdd);
            return false;
        }
    }
    m_objects.append(pObject);
    return true;
}

/**
 * Removes an object from this package.
 * Does not physically delete the object.
 * Does not emit signals.
 *
 * @param pObject   Pointer to the UMLObject to be removed.
 */
void UMLPackage::removeObject(UMLObject *pObject)
{
    if (pObject->baseType() == UMLObject::ot_Association) {
        UMLAssociation *assoc = pObject->asUMLAssociation();
        if (assoc)
	    removeAssocFromConcepts(assoc);
        else
            logError0("UMLPackage::removeObject: object asserts to be UMLAssociation but is not");
    }
    if (m_objects.indexOf(pObject) == -1) {
        logDebug2("UMLPackage %1 removeObject: object with id=%2 not found.",
                  name(), Uml::ID::toString(pObject->id()));
        return;
    }
    // Do not delete a datatype from its root folder but just mark it as inactive.
    //   https://bugs.kde.org/show_bug.cgi?id=427532
    // We need this special handling because of the possibility of switching
    // the active programming language.  Without it,
    // - switching the active language could create dangling references on all
    //   objects referencing the previous language's datatypes;
    // - the display of the datatypes in the list view or in dialogs get
    //   populated with the types from previously active languages.
    if (this == UMLApp::app()->document()->datatypeFolder()) {
        UMLDatatype *dt = pObject->asUMLDatatype();
        if (dt) {
            dt->setActive(false);
        } else {
            logWarn2("UMLPackage::removeObject(%1) : Expected Datatype, found %2",
                     pObject->name(), pObject->baseTypeStr());
        }
    } else {
        m_objects.removeAll(pObject);
    }
}

/**
 * Removes all objects from this package.
 * Inner containers (e.g. nested packages) are removed recursively.
 */
void UMLPackage::removeAllObjects()
{
    UMLCanvasObject::removeAllChildObjects();

    for (int i = 0; i < m_objects.size(); i++) {
        UMLObject *o = m_objects.at(i);
        uIgnoreZeroPointer(o);
        UMLPackage *pkg = o->asUMLPackage();
        if (pkg)
            pkg->removeAllObjects();
        removeObject(o);
        delete o;
    }
    m_objects.clear();
}

/**
 * Returns the list of objects contained in this package.
 */
UMLObjectList UMLPackage::containedObjects(bool includeInactive /* = false */) const
{
    UMLObjectList result;
    for(UMLObject *obj : m_objects) {
        uIgnoreZeroPointer(obj);
        if (includeInactive) {
            result.append(obj);
        }
        else if (obj->isUMLDatatype()) {
            UMLDatatype *dt = obj->asUMLDatatype();
            if (dt->isActive())
                result.append(obj);
        }
        else {
            result.append(obj);
        }
    }
    return result;
}

/**
 * Find the object of the given name in the list of contained objects.
 *
 * @param name   The name to seek.
 * @return  Pointer to the UMLObject found or NULL if not found.
 */
UMLObject * UMLPackage::findObject(const QString &name) const
{
    const bool caseSensitive = UMLApp::app()->activeLanguageIsCaseSensitive();
    for(UMLObject *obj : m_objects) {
        uIgnoreZeroPointer(obj);
        if (!obj)
            continue;
        if (caseSensitive) {
            if (obj->name() == name)
                return obj;
        } else if (obj->name().toLower() == name.toLower()) {
            return obj;
        }
    }
    return nullptr;
}

/**
 * Find the object of the given ID in the list of contained objects.
 *
 * @param id   The ID to seek.
 * @return  Pointer to the UMLObject found or NULL if not found.
 */
UMLObject * UMLPackage::findObjectById(Uml::ID::Type id) const
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
void UMLPackage::appendPackages(UMLPackageList& packages, bool includeNested) const
{
    for(UMLObject *o : m_objects) {
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
                                   bool includeNested /* = true */) const
{
    for(UMLObject *o : m_objects) {
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
                                 bool includeNested /* = true */) const
{
    for(UMLObject *o :  m_objects) {
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
 * Append all classes, interfaces, and enums from this package (and those
 * from nested packages) to the given UMLClassifierList.
 *
 * @param classifiers     The list to append to.
 * @param includeNested   Whether to include the classifiers from
 *                        nested packages (default: true.)
 */
void UMLPackage::appendClassesAndInterfaces(UMLClassifierList& classifiers,
        bool includeNested /* = true */) const
{
    for(UMLObject *o : m_objects) {
        uIgnoreZeroPointer(o);
        ObjectType ot = o->baseType();
        if (ot == ot_Class || ot == ot_Interface || ot == ot_Enum) {
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
    for(UMLObject *obj : m_objects) {
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
void UMLPackage::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("Package"));
    if (! Settings::optionState().generalState.uml2) {
        writer.writeStartElement(QStringLiteral("UML:Namespace.ownedElement"));
    }
    // save classifiers etc.
    for(UMLObject *obj : m_objects) {
        uIgnoreZeroPointer(obj);
        obj->saveToXMI (writer);
    }
    // save associations
    for(UMLObject *obj: subordinates()) {
        uIgnoreZeroPointer(obj);
        obj->saveToXMI (writer);
    }
    if (! Settings::optionState().generalState.uml2) {
        writer.writeEndElement();            // UML:Namespace.ownedElement
    }
    UMLObject::save1end(writer); // UML:Package
}

/**
 * Loads the <UML:Package> XMI element.
 * Auxiliary to UMLObject::loadFromXMI.
 */
bool UMLPackage::load1(QDomElement& element)
{
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (Model_Utils::isCommonXMI1Attribute(type))
            continue;
        if (UMLDoc::tagEq(type, QStringLiteral("Namespace.ownedElement")) ||
                UMLDoc::tagEq(type, QStringLiteral("Element.ownedElement")) ||  // Embarcadero's Describe
                UMLDoc::tagEq(type, QStringLiteral("Namespace.contents"))) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load1(tempElement))
                return false;
            continue;
        } else if (UMLDoc::tagEq(type, QStringLiteral("packagedElement")) ||
                   UMLDoc::tagEq(type, QStringLiteral("ownedElement"))) {
            type = tempElement.attribute(QStringLiteral("xmi:type"));
        }
        QString stereoID = tempElement.attribute(QStringLiteral("stereotype"));
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type, stereoID);
        if (!pObject) {
            logWarn1("UMLPackage::load1 unknown type of umlobject to create: %1", type);
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


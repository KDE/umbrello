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

#ifndef PACKAGE_H
#define PACKAGE_H

#include "umlcanvasobject.h"
#include "umlclassifierlist.h"

// forward declarations
class UMLAssociation;


/**
 * This class contains the non-graphical information required for a UML
 * Package.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * @short Non-graphical information for a Package.
 * @author Jonathan Riddell
 * @see UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLPackage : public UMLCanvasObject {
    Q_OBJECT
public:
    /**
     * Sets up a Package.
     *
     * @param name              The name of the Concept.
     * @param id                The unique id of the Concept.
     */
    explicit UMLPackage(const QString & name = "", Uml::IDType id = Uml::id_None);

    /**
     * Empty deconstructor.
     */
    virtual ~UMLPackage();

    /**
     * Copy the internal presentation of this object into the new
     * object.
     */
    virtual void copyInto(UMLPackage *rhs) const;

    /**
     * Make a clone of this object.
     */
    virtual UMLObject* clone() const;

    /**
     * Initializes key variables of the class.
     */
    void init();

    /**
     * Adds an object in this package.
     *
     * @param pObject   Pointer to the UMLObject to add.
     * @return    True if the object was actually added.
     */
    bool addObject(UMLObject *pObject);

    /**
     * Removes an object from this package.
     * Does not physically delete the object.
     *
     * @param pObject   Pointer to the UMLObject to be removed.
     */
    void removeObject(UMLObject *pObject);

    /**
     * Removes all objects from this package.
     * Inner containers (e.g. nested packages) are removed recursively.
     */
    virtual void removeAllObjects();

    /**
     * Returns the list of objects contained in this package.
     */
    UMLObjectList containedObjects();

    /**
     * Adds an existing association to the matching concept in the list of concepts.
     * The selection of the matching concept depends on the association type:
     * For generalizations, the assoc is added to the concept that matches role A.
     * For aggregations and compositions , the assoc is added to the concept
     * that matches role B.
     *
     * @param assoc     The association to add
     */
    void addAssocToConcepts(UMLAssociation* assoc);

    /**
     * Remove the association from the participating concepts.
     */
    void removeAssocFromConcepts(UMLAssociation *assoc);

    /**
     * Find the object of the given name in the list of contained objects.
     *
     * @param name              The name to seek.
     * @return  Pointer to the UMLObject found or NULL if not found.
     */
    UMLObject * findObject(const QString &name);

    /**
     * Find the object of the given ID in the list of contained objects.
     *
     * @param id                The ID to seek.
     * @return  Pointer to the UMLObject found or NULL if not found.
     */
    UMLObject * findObjectById(Uml::IDType id);

    /**
     * Append all classifiers from this package (and those from
     * nested packages) to the given UMLClassifierList.
     *
     * @param classifiers               The list to append to.
     * @param includeNested             Whether to include the classifiers from
     *                          nested packages (default: true.)
     */
    void appendClassifiers( UMLClassifierList& classifiers,
                            bool includeNested = true );

    /**
     * Append all classes from this package (and those from
     * nested packages) to the given UMLClassifierList.
     *
     * @param classes           The list to append to.
     * @param includeNested             Whether to include the classes from
     *                          nested packages (default: true.)
     */
    void appendClasses( UMLClassifierList& classes, bool includeNested = true );

    /**
     * Append all classes and interfaces from this package (and those
     * from nested packages) to the given UMLClassifierList.
     *
     * @param classifiers               The list to append to.
     * @param includeNested             Whether to include the classifiers from
     *                          nested packages (default: true.)
     */
    void appendClassesAndInterfaces(UMLClassifierList& classifiers,
                                    bool includeNested = true);

    /**
     * Append all interfaces from this package (and those from
     * nested packages) to the given UMLClassifierList.
     *
     * @param interfaces                The list to append to.
     * @param includeNested             Whether to include the interfaces from
     *                          nested packages (default: true.)
     */
    void appendInterfaces(UMLClassifierList& interfaces,
                          bool includeNested = true );

    /**
     * Resolve types. Required when dealing with foreign XMI files.
     * Needs to be called after all UML objects are loaded from file.
     * Overrides the method from UMLObject.
     * Calls resolveRef() on each contained object.
     *
     * @return  True for overall success.
     */
    virtual bool resolveRef();

    /**
     * Creates the <UML:Package> XMI element.
     */
    virtual void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
    /**
     * Loads the <UML:Package> XMI element.
     * Auxiliary to UMLObject::loadFromXMI.
     */
    virtual bool load(QDomElement& element);

    /**
     * References to the objects contained in this package.
     * The UMLPackage is the owner of the objects.
     */
    UMLObjectList m_objects;

};

#endif

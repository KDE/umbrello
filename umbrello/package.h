/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PACKAGE_H
#define PACKAGE_H

#include "umlcanvasobject.h"
#include "umlclassifierlist.h"
#include "umlinterfacelist.h"

class IDChangeLog;

/**
 * This class contains the non-graphical information required for a UML
 * Package.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * The @ref UMLDoc class creates instances of this type.  All Packages will
 * need a unique id.  This will be given by the @ref UMLDoc class.
 * If you don't leave it up to the @ref UMLDoc class then call the method
 * @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Non-graphical information for a Package.
 * @author Jonathan Riddell
 * @see	UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLPackage : public UMLCanvasObject {
	Q_OBJECT
public:
	/**
	 * Sets up a Package.
	 *
	 * @param name		The name of the Concept.
	 * @param id		The unique id of the Concept.
	 */
	UMLPackage(const QString & Name = "", int id = -1);

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
	 * @param pObject	Pointer to the UMLObject to add.
	 */
	void addObject(const UMLObject *pObject);

	/**
	 * Removes an object from this package.
	 *
	 * @param pObject	Pointer to the UMLObject to be removed.
	 */
	void removeObject(const UMLObject *pObject);

	/**
	 * Returns the list of objects contained in this package.
	 */
	UMLObjectList& containedObjects();

	/**
	 * Find the object of the given name in the list of contained objects.
	 *
	 * @param name		The name to seek.
	 * @return	Pointer to the UMLObject found or NULL if not found.
	 */
	UMLObject * findObject(QString name);

	/**
	 * Find the object of the given ID in the list of contained objects.
	 *
	 * @param id		The ID to seek.
	 * @return	Pointer to the UMLObject found or NULL if not found.
	 */
	UMLObject * findObject(int id);

	/**
	 * Find the object of the given (non-numeric) auxiliary ID
	 * in the list of contained objects. The auxiliary ID is the
	 * ID returned by UMLObject::getAuxId() and is currently
	 * only used for loading foreign XMI files.
	 *
	 * @param idStr		The ID to seek.
	 * @return	Pointer to the UMLObject found or NULL if not found.
	 */
	UMLObject* findObjectByIdStr(QString idStr);

	/**
	 * Append all classifiers from this package (and those from
	 * nested packages) to the given UMLClassifierList.
	 *
	 * @param classifiers		The list to append to.
	 * @param includeNested		Whether to include the classifiers from
	 *				nested packages (default: true.)
	 */
	void appendClassifiers( UMLClassifierList& classifiers,
				bool includeNested = true );

	/**
	 * Append all classes and interfaces from this package (and those
	 * from nested packages) to the given UMLClassifierList.
	 *
	 * @param classifiers		The list to append to.
	 * @param includeNested		Whether to include the classifiers from
	 *				nested packages (default: true.)
	 */
	void appendClassesAndInterfaces(UMLClassifierList& classifiers,
					bool includeNested = true);

	/**
	 * Append all interfaces from this package (and those from
	 * nested packages) to the given UMLInterfaceList.
	 *
	 * @param interfaces		The list to append to.
	 * @param includeNested		Whether to include the interfaces from
	 *				nested packages (default: true.)
	 */
	void appendInterfaces(UMLInterfaceList& interfaces,
				bool includeNested = true );

	/**
	 * Creates the <UML:Package> XMI element.
	 */
	void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
	/**
	 * Loads the <UML:Package> XMI element.
	 * Auxiliary to UMLObject::loadFromXMI.
	 */
	bool load(QDomElement& element);

private:
	/**
	 * References to the objects contained in this package.
	 * The UMLPackage is the owner of the objects.
	 */
	UMLObjectList m_objects;

};

#endif

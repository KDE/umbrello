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

#include <qptrlist.h>
#include "umlcanvasobject.h"

class IDChangeLog;
class UMLAssociation;
class UMLDoc;

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
	 * @param parent	The parent to this Concept.
	 * @param name		The name of the Concept.
	 * @param id		The unique id of the Concept.
	 */
	UMLPackage(UMLDoc * parent, const QString & Name = "", int id = -1);

	/**
	 * Empty deconstructor.
	 */
	virtual ~UMLPackage();

	/**
	 * Initializes key variables of the class.
	 */
	void init();

	/**
	 * Creates the <UML:Package> XMI element.
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads the <UML:Package> XMI element.
	 */
	bool loadFromXMI(QDomElement& element);

private:

};

#endif

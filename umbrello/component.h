/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMPONENT_H
#define COMPONENT_H

#include "umlcanvasobject.h"

class IDChangeLog;
class UMLDoc;

/**
 * This class contains the non-graphical information required for a
 * UML Component.
 * This class inherits from @ref UMLCanvasObject which contains most
 * of the information.
 *
 * The @ref UMLDoc class creates instances of this type.
 * All Components will need a unique id.  This will be given by the
 * @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 * class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Non-graphical information for a Component.
 * @author Jonathan Riddell
 * @see	UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLComponent : public UMLCanvasObject {
	Q_OBJECT
public:
	/**
	 * Sets up a Component.
	 *
	 * @param parent	The parent to this Concept.
	 * @param name		The name of the Concept.
	 * @param id		The unique id of the Concept.
	 */
	UMLComponent(UMLDoc* parent, const QString & name = "", int id = -1);

	/**
	 * Empty deconstructor.
	 */
	virtual ~UMLComponent();

	/**
	 * Initializes key variables of the class.
	 */
	virtual void init();

	/**
	 * Creates the UML:Component element including it's operations,
	 * attributes and templates
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the UML:Component element including it's operations,
	 * attributes and templates
	 */
	bool loadFromXMI( QDomElement & element );

	/**
	 * sets m_executable
	 */
	void setExecutable(bool executable);

	/**
	 * returns the value of m_executable
	 */
	bool getExecutable();
private:
	
	/**
	 * holds whether this is an executable component or not
	 */
	bool m_executable;
};

#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef NODE_H
#define NODE_H

#include "umlcanvasobject.h"

class IDChangeLog;

/**
 * This class contains the non-graphical information required for a UML Node.
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * The @ref UMLDoc class creates instances of this type.  All Nodes will
 * need a unique id.  This will be given by the @ref UMLDoc class.
 * If you don't leave it up to the @ref UMLDoc class then call the method
 * @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Non-graphical information for a Node.
 * @author Jonathan Riddell
 * @see	UMLCanvasObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLNode : public UMLCanvasObject {
	Q_OBJECT
public:
	/**
	 * Sets up a Node.
	 *
	 * @param name		The name of the Concept.
	 * @param id		The unique id of the Concept.
	 */
	UMLNode(const QString & name = "", int id = -1);

	/**
	 * Empty deconstructor.
	 */
	virtual ~UMLNode();

	/**
	 * Initializes key variables of the class.
	 */
	virtual void init();

	/**
	 * Creates the <UML:Node> XMI element including its operations,
	 * attributes and templates.
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <UML:Node> XMI element including its operations,
	 * attributes and templates.
	 */
	bool loadFromXMI( QDomElement & element );

private:
	
};

#endif

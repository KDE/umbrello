/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef INTERFACE_H
#define INTERFACE_H

#include "classifier.h"

/**
 *	This class contains the non-graphical information required for a UML Interface.
 *	This class inherits from @ref UMLClassifier which contains most of the information.
 *	The @ref UMLDoc class creates instances of this type.  All Interfaces will need a unique
 *	id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 *	class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 *	@short	Information for a non-graphical Interface.
 *	@author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLInterface : public UMLClassifier {
public:
	/**
	 *	Sets up an interface.
	 *
	 *	@param	parent	The parent to this Concept.
	 *	@param	name	The name of the Concept.
	 *	@param	id	The unique id of the Concept.
	 */
	UMLInterface(QObject* parent, QString Name, int id);

	/**
	 *	Sets up an interface.
	 *
	 *	@param	parent		The parent to this Concept.
	 */
	UMLInterface(QObject * parent);

	/**
	 *	Standard deconstructor.
	 */
	virtual ~UMLInterface();

	/**
	 * 		Overloaded '==' operator
	 */
  	bool operator==(UMLInterface& rhs);
  
	/**
	 * Returns a name for the new association, operation, template
	 * or attribute appended with a number if the default name is
	 * taken e.g. new_association, new_association_1 etc
	 */
	virtual QString uniqChildName(const UMLObject_Type type);

	/**
	 * Creates the UML:Concept element including it's operations, attributes and templates
	 */
	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the UML:Class element including it's operations, attributes and templates
	 */
	virtual bool loadFromXMI( QDomElement & element );

protected:

	/**
	 *	Initializes key variables of the class.
	 */
	virtual void init(); // doesnt seem to be any reason for this to be public 
};

#endif // INTERFACE_H 

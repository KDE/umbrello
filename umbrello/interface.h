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
 * This class contains the non-graphical information required for a UML
 * Interface.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * The @ref UMLDoc class creates instances of this type.  All Interfaces
 * will need a unique id.  This will be given by the @ref UMLDoc class.
 * If you don't leave it up to the @ref UMLDoc class then call the method
 * @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Information for a non-graphical Interface.
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLInterface : public UMLClassifier {
	Q_OBJECT
public:
	/**
	 * Sets up an interface.
	 *
	 * @param name		The name of the Interface.
	 * @param id		The unique id of the Interface.
	 */
	UMLInterface(const QString & name = "", int id = -1);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLInterface();

	/**
	 * Overloaded '==' operator.
	 */
  	bool operator==(UMLInterface& rhs);

	/**
	 * Copy the internal presentation of this object into the new
	 * object.
	 */
	virtual void copyInto(UMLInterface *rhs) const;

	/**
	 * Make a clone of this object.
	 */
	virtual UMLObject* clone() const;


	/**
	 * Returns a name for the new association, operation, template
	 * or attribute appended with a number if the default name is
	 * taken e.g. new_association, new_association_1 etc
	 */
	virtual QString uniqChildName(const UMLObject_Type type);

	virtual bool isInterface () { return true; }

	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
	/**
	 * Loads the <UML:Interface> XMI element including its operations.
	 */
	bool load( QDomElement & element );

	/**
	 * Initializes key variables of the class.
	 */
	virtual void init();
};

#endif // INTERFACE_H

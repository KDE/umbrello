/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DATATYPE_H
#define DATATYPE_H

#include "classifier.h"

class UMLObject;

/**
 * This class contains the non-graphical information required for a UML
 * Datatype.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * The @ref UMLDoc class creates instances of this type.  All Datatypes
 * will need a unique id.  This will be given by the @ref UMLDoc class.
 * If you don't leave it up to the @ref UMLDoc class then call the method
 * @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Information for a non-graphical Datatype.
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLDatatype : public UMLClassifier {
public:
	/**
	 * Sets up an datatype.
	 *
	 * @param name		The name of the Concept.
	 * @param id		The unique id of the Concept.
	 */
	UMLDatatype(const QString & name = "", int id = -1);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLDatatype();

	/**
	 * Overloaded '==' operator.
	 */
	bool operator==(UMLDatatype& rhs);

	/**
	 * Copy the internal presentation of this object into the new
	 * object.
	 */
	virtual void copyInto(UMLDatatype *rhs) const;

	/**
	 * Make a clone of this object.
	 */
	virtual UMLObject* clone() const;

	/**
	 * Creates the <UML:DataType> XMI element.
	 */
	void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * returns false
	 */
	virtual bool isInterface();

	/**
	 * Set the origin type (in case of e.g. typedef)
	 */
	void setOriginType(UMLClassifier *origType);

	/**
	 * Get the origin type (in case of e.g. typedef)
	 */
	UMLClassifier * originType();

	/**
	 * Set the m_isRef flag (true when dealing with a pointer type)
	 */
	void setIsReference(bool isRef = true);

	/**
	 * Get the m_isRef flag.
	 */
	bool isReference();

protected:
	/**
	 * Loads the <UML:DataType> XMI element (empty.)
	 */
	bool load( QDomElement & element );

	/**
	 * Initializes key variables of the class.
	 */
	virtual void init(); // doesnt seem to be any reason for this to be public

	UMLClassifier *m_pOrigType;
	bool m_isRef;
};

#endif // DATATYPE_H

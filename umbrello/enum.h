 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENUM_H
#define ENUM_H

#include "classifier.h"
#include "umlenumliterallist.h"
#include "umlclassifierlistitemlist.h"

/**
 * This class contains the non-graphical information required for a UML
 * Enum.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * The @ref UMLDoc class creates instances of this type.  All Enums
 * will need a unique id.  This will be given by the @ref UMLDoc class.
 * If you don't leave it up to the @ref UMLDoc class then call the method
 * @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Non-graphical Information for an Enum.
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLEnum : public UMLClassifier {
	Q_OBJECT
public:
	/**
	 * Sets up an enum.
	 *
	 * @param name		The name of the Enum.
	 * @param id		The unique id of the Enum.
	 */
	UMLEnum(const QString& name = "", Uml::IDType id = Uml::id_None);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLEnum();

	/**
	 * Overloaded '==' operator.
	 */
  	bool operator==(UMLEnum& rhs);

	/**
	 * Copy the internal presentation of this object into the new
	 * object.
	 */
	virtual void copyInto(UMLEnum *rhs) const;

	/**
	 * Make a clone of this object.
	 */
	virtual UMLObject* clone() const;

	/**
	 * Creates the <UML:Enum> element including its enumliterals.
	 */
	virtual void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	virtual bool isInterface () { return false; }

 	/**
  	 * Adds an enumliteral to the enum.
  	 *
  	 * @param name		The name of the enumliteral.
	 * @param id		The id of the enumliteral (optional.)
	 *			If omitted a new ID is assigned internally.
	 * @return	Pointer to the UMLEnumliteral created.
	 */
	UMLObject* addEnumLiteral(QString name, Uml::IDType id = Uml::id_None);

	/**
	 * Adds an already created enumliteral.
	 * The enumliteral object must not belong to any other concept.
  	 *
  	 * @param Att		Pointer to the UMLEnumLiteral.
	 * @param Log		Pointer to the IDChangeLog.
	 * @return	True if the enumliteral was sucessfully added.
	 */
	bool addEnumLiteral(UMLEnumLiteral* Att, IDChangeLog* Log = 0);

	/**
	 * Adds an enumliteral to the enum, at the given position.
	 * If position is negative or too large, the enumliteral is added
	 * to the end of the list.
  	 *
  	 * @param Att		Pointer to the UMLEnumLiteral.
	 * @param position	Position index for the insertion.
	 * @return	True if the enumliteral was sucessfully added.
	 */
	 //TODO:  give default value -1 to position (append) - now it conflicts with the method above..
	bool addEnumLiteral(UMLEnumLiteral* Att, int position );

	/**
	 * Removes an enumliteral from the class.
	 *
	 * @param a		The enumliteral to remove.
	 * @return	Count of the remaining enumliterals after removal.
	 *		Returns -1 if the given enumliteral was not found.
	 */
	int removeEnumLiteral(UMLObject *a);
	/**
	 * Take and return an enumliteral from class.
	 * It is the callers responsibility to pass on ownership of
	 * the returned enumliteral (or to delete the enumliteral)
	 *
	 * @param  el enumliteral to tkae
	 * @return pointer to the enumliteral or null if not found.
	 */
	UMLEnumLiteral* takeEnumLiteral(UMLEnumLiteral* el);

	/**
	 * Returns the number of enumliterals for the class.
	 *
	 * @return	The number of enumliterals for the class.
	 */
	int enumLiterals();

	/**
	 * Find a list of enumliterals with the given name.
	 *
	 * @param t		The type to find.
	 * @param n		The name of the object to find.
	 * @return	List of objects found.  Will be empty if none found.
	 */
	 virtual UMLObjectList findChildObject(Uml::Object_Type t, QString n);

signals:
	void enumLiteralAdded(UMLObject*);
	void enumLiteralRemoved(UMLObject*);

protected:
	/**
	 * Loads the <UML:Enum> element including its enumliterals.
	 */
	bool load( QDomElement & element );

private:
	/**
	 * Initializes key variables of the class.
	 */
	void init();

};

#endif // ENUM_H


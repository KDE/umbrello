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

class UMLDoc;
class UMLEnumLiteral;

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
	 * @param parent	The parent to this Enum.
	 * @param name		The name of the Enum.
	 * @param id		The unique id of the Enum.
	 */
	UMLEnum(UMLDoc* parent, const QString& name = "", int id = -1);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLEnum();

	/**
	 * Overloaded '==' operator.
	 */
  	bool operator==(UMLEnum& rhs);
  
	/**
	 * Returns a name for the new enumliteral
	 * appended with a number if the default name is
	 * taken e.g. new_literal, new_literal_1 etc
	 */
	virtual QString uniqChildName(const UMLObject_Type type);

	/**
	 * Creates the <UML:Enum> element including its enumliterals.
	 */
	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <UML:Enum> element including its enumliterals.
	 */
	virtual bool loadFromXMI( QDomElement & element );

	virtual bool isInterface () { return false; }

 	/**
  	 * Adds an enumliteral to the enum.
  	 *
  	 * @param name		The name of the enumliteral.
	 * @param id		The id of the enumliteral.
	 * @return	Pointer to the UMLEnumliteral created.
	 */
	UMLObject* addEnumLiteral(QString name, int id);

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
	 * Returns the number of enumliterals for the class.
	 *
	 * @return	The number of enumliterals for the class.
	 */
	int enumLiterals();

	/**
	 * Return the list of enumliterals for the class.
	 *
	 * @return	Pointer to the list of enumliterals for the class.
	 */
	QPtrList<UMLClassifierListItem>* getEnumLiteralList();

	/**
	 * Returns the entries in m_pAttList that are actually enumliterals.
	 *
	 * @return	Pointer to the list of true enumliterals for the class.
	 */
	UMLEnumLiteralList* getFilteredEnumLiteralList();

	/**
	 * Find a list of enumliterals with the given name.
	 *
	 * @param t		The type to find.
	 * @param n		The name of the object to find.
	 * @return	List of objects found.  Will be empty if none found.
	 */
	 virtual UMLObjectList findChildObject(UMLObject_Type t, QString n);

	/**
	 * Find an enumliteral
	 *
	 * @param id		The id of the object to find.
	 * @return	Pointer to the object found; NULL if not found.
	 */
	virtual UMLObject* findChildObject(int id);

	/**
	 * Add an already created stereotype to the list identified by the
	 * given UMLObject_type.
	 *
	 * @param newStereotype	Pointer to the UMLStereotype to add.
	 * @param list		The object type for the list on which to add.
	 * @param log		Pointer to the IDChangeLog.
	 * @return	True if the newStereotype was successfully added.
	 */
	virtual bool addStereotype(UMLStereotype* newStereotype, UMLObject_Type list, IDChangeLog* log = 0);

signals:
	void enumLiteralAdded(UMLObject*);
	void enumLiteralRemoved(UMLObject*);

protected:

	/**
	 * Initializes key variables of the class.
	 */
	virtual void init();

	/**
	 * List of all the enumliterals in this class.
	 */
	QPtrList<UMLClassifierListItem> m_EnumLiteralList;

};

#endif // ENUM_H 

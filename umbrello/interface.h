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

#include "umlcanvasobject.h"
#include <qptrlist.h>

class IDChangeLog;
class UMLAssociation;
class UMLOperation;

/**
 *	This class contains the non-graphical information required for a UML Interface.
 *	This class inherits from @ref UMLCanvasObject which contains most of the information.
 *	The @ref UMLDoc class creates instances of this type.  All Interfaces will need a unique
 *	id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 *	class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 *	@short	Information for a non-graphical Interface.
 *	@author Jonathan Riddell
 */

class UMLInterface : public UMLCanvasObject {
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
	 *	Adds an operation to the Interface.
	 *
	 *	@param	name	The name of the operation.
	 *	@param	id	The id of the operation.
	 */
	UMLObject* addOperation(QString name, int id);

	/**
	 *	Adds an already created Operation. The Operation must not belong to any other
	 *	interface
	 */
	bool addOperation(UMLOperation* Op);

	/**
	 *	Adds an already created Operation and checks for
	 *	operations with the same name. The Operation must not
	 *	belong to any other interface.  Used by the clipboard
	 *	when pasteing.
	 */
	bool addOperation(UMLOperation* Op, IDChangeLog* Log);

	/**
	 *	Remove an operation from the interface.
	 *
	 *	@param	o	The operation to remove.
	 */
	int removeOperation(UMLObject *o);

	/**
	 *	Return the list of operations for the Interface.
	 *
	 *	@return The list of operation for the Interface.
	 */
	QPtrList<UMLOperation>* getOpList();

	/**
	 *	Find a list of operations or associations 
	 *	with the given name.
	 *
	 *	@param	t	The type to find.
	 *	@param	n	The name of the object to find.
	 *
	 *	@return	The object found.  Will return 0 if none found.
	 */
	QPtrList<UMLObject> findChildObject(UMLObject_Type t, QString n);

	/**
	 *	Find an attribute, operation, association or template.
	 *
	 *	@param	id	The id of the object to find.
	 *
	 *	@return	The object found.  Will return 0 if none found.
	 */
	UMLObject* findChildObject(int id);

	/**
	 *	Returns the number of operations for the interface.
	 *
	 *	@return	The number of operations for the interface.
	 */
	int operations() ;

	/**
	 * Used to save or load this classes information for the clipboard
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 * 								load the information.
	 *	@param	fileversion	the version of the serialize format
	 *
	 *	@return	Returns the result of the operation.
	 */
	virtual bool serialize(QDataStream* s, bool archive, int fileversion);

	/**
	 * Returns the amount of bytes needed to serialize an instance object to the clipboard
	 */
	virtual long getClipSizeOf();

	/**
	 *	Initializes key variables of the class.
	 */
	virtual void init();

	/**
	 * Creates the UML:Interface element including it's operations,
	 * attributes and templates
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads the UML:Interface element including it's operations,
	 * attributes and templates
	 */
	bool loadFromXMI(QDomElement& element);

private:
	/**
	 * 	List of all the operations in this interface.
	 */
	QPtrList<UMLOperation> m_OpsList;

};

#endif

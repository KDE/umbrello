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

#include "umlobject.h"
#include <qptrlist.h>

class IDChangeLog;
class UMLAssociation;
class UMLOperation;

/**
 *	This class contains the non-graphical information required for a UML Interface.
 *	This class inherits from @ref UMLObject which contains most of the information.
 *	The @ref UMLDoc class creates instances of this type.  All Interfaces will need a unique
 *	id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 *	class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 *	@short	Information for a non-graphical Interface.
 *	@author Jonathan Riddell
 *	@see	UMLObject
 */

class UMLInterface : public UMLObject {
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
 	 *	Adds an association.
 	 *	Which role is "this" side (i.e. identifies the current concept)
 	 *	depends on the association type:
 	 *	For generalizations, role A is "this" side.
 	 *	For aggregations and compositions, role B is "this" side.
 	 *
 	 *	@param	assoc	The association to add.
 	 */
 	bool addAssociation(UMLAssociation* assoc);
 
 	/**
 	 *	Determine if this interface has this association.
 	 *	@param	assoc	The association to check.
 	 */
 	bool hasAssociation(UMLAssociation* assoc);
 
 	/**
 	 *	Remove an association from the Interface.
 	 *
 	 *	@param	o	The association to remove.
 	 */
 	int removeAssociation(UMLObject *o);
 
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
	 *	Returns the number of associations for the interface.
	 *	This is the sum of the aggregations and compositions.
	 *
	 *	@return	The number of associations for the interface.
	 */
	int associations();

	/**
	 *	Returns the number of operations for the interface.
	 *
	 *	@return	The number of operations for the interface.
	 */
	int operations() ;

 	/**
	 *	Return the list of associations for the interface.
	 *
	 *	@return The list of associations for the Interface.
	 */
	const QPtrList<UMLAssociation>& getAssociations();

	/**
	 *	Return the subset of m_AssocsList that matches `assocType'.
	 *
	 *	@return The list of associations that match `assocType'.
	 */
	QPtrList<UMLAssociation> getSpecificAssocs(Uml::Association_Type assocType);

	/**FIXME needed for interfaces?
	 *	Shorthand for getSpecificAssocs(Uml::at_Generalization)
	 *
	 *	@return The list of generalizations for the Interface.
	 */
	QPtrList<UMLAssociation> getGeneralizations();

	/**FIXME needed for interfaces?
	 *	Shorthand for getSpecificAssocs(Uml::at_Aggregation)
	 *
	 *	@return The list of aggregations for the Interface.
	 */
	QPtrList<UMLAssociation> getAggregations();

	/**FIXME needed?
	 *	Shorthand for getSpecificAssocs(Uml::at_Composition)
	 *
	 *	@return The list of compositions for the Interface.
	 */
	QPtrList<UMLAssociation> getCompositions();

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
	 * Used to save or load this classes information for the clipboard
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 * load the information.
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
	void init();

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

	/**
	 * Returns a name for the new operation or
	 * association appended with a number if the
	 * default name is taken e.g. new attribute, new attribute_1
	 * etc
	 */
	QString uniqChildName(const UMLObject_Type type);

private:
	/**
	 * 	List of all the associations in this interface.
	 */
	QPtrList<UMLAssociation> m_AssocsList;

	/**
	 * 	List for computation of subsets of m_AssocsList.
	 * 	This is always computed from m_AssocsList.
	 */
	QPtrList<UMLAssociation> m_TmpAssocs;

	/**
	 * 	List of all the operations in this interface.
	 */
	QPtrList<UMLOperation> m_OpsList;

};

#endif

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

#include "umlobject.h"
#include <qptrlist.h>

class IDChangeLog;
class UMLAssociation;

/**
 *	This class contains the non-graphical information required for a UML Component.
 *	This class inherits from @ref UMLObject which contains most of the information.
 *	The @ref UMLDoc class creates instances of this type.  All Components will need a unique
 *	id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 *	class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 *	@short	Non-graphical information for a Component.
 *	@author Jonathan Riddell
 *	@see	UMLObject
 */

class UMLComponent : public UMLObject {
public:
	/**
	 *	Sets up a Component.
	 *
	 *	@param	parent	The parent to this Concept.
	 *	@param	name	The name of the Concept.
	 *	@param	id	The unique id of the Concept.
	 */
	UMLComponent(QObject* parent, QString Name, int id);

	/**
	 *	Sets up a Component.
	 *
	 *	@param	parent		The parent to this Concept.
	 */
	UMLComponent(QObject* parent);

	/**
	 *	Standard deconstructor.
	 */
	virtual ~UMLComponent();

	/**
	 * 		Overloaded '==' operator
	 */
  	bool operator==(UMLComponent& rhs);
  
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
 	 *	Determine if this component has this association.
 	 *	@param	assoc	The association to check.
 	 */
 	bool hasAssociation(UMLAssociation* assoc);
 
 	/**
 	 *	Remove an association from the Component.
 	 *
 	 *	@param	o	The association to remove.
 	 */
 	int removeAssociation(UMLObject *o);
 
	/**
	 *	Returns the number of associations for the Component.
	 *	This is the sum of the aggregations and compositions.
	 *
	 *	@return	The number of associations for the Concept.
	 */
	int associations();

 	/**
	 *	Return the list of associations for the Component.
	 *
	 *	@return The list of associations for the Component.
	 */
	const QPtrList<UMLAssociation>& getAssociations();

	/**
	 *	Return the subset of m_AssocsList that matches `assocType'.
	 *
	 *	@return The list of associations that match `assocType'.
	 */
	QPtrList<UMLAssociation> getSpecificAssocs(Uml::Association_Type assocType);

	/**
	 *	Shorthand for getSpecificAssocs(Uml::at_Generalization)
	 *
	 *	@return The list of generalizations for the Concept.
	 */
	QPtrList<UMLAssociation> getGeneralizations();

	/**
	 *	Shorthand for getSpecificAssocs(Uml::at_Aggregation)
	 *
	 *	@return The list of aggregations for the Concept.
	 */
	QPtrList<UMLAssociation> getAggregations();

	/**
	 *	Shorthand for getSpecificAssocs(Uml::at_Composition)
	 *
	 *	@return The list of compositions for the Concept.
	 */
	QPtrList<UMLAssociation> getCompositions();

	/**
	 *	Find a list of associations with the given name.
	 *
	 *	@param	t	The type to find.
	 *	@param	n	The name of the object to find.
	 *
	 *	@return	The object found.  Will return 0 if none found.
	 */
	QPtrList<UMLObject> findChildObject(UMLObject_Type t, QString n);

	/**
	 *	Find an association.
	 *
	 *	@param	id	The id of the object to find.
	 *
	 *	@return	The object found.  Will return 0 if none found.
	 */
	UMLObject* findChildObject(int id);

	/**
	 * Use to save or load this class's information for the clipboard.
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
	 * Returns a name for the new association appended with a
	 * number if the default name is taken e.g. new_association,
	 * new_association_1 etc
	 */
	QString uniqChildName(const UMLObject_Type type);

private:
	/**
	 * 	List of all the associations in this class.
	 */
	QPtrList<UMLAssociation> m_AssocsList;

	/**
	 * 	List for computation of subsets of m_AssocsList.
	 * 	This is always computed from m_AssocsList.
	 */
	QPtrList<UMLAssociation> m_TmpAssocs;

};

#endif

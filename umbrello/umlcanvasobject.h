/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CANVASOBJECT_H
#define CANVASOBJECT_H

#include "umlobject.h"
#include <qptrlist.h>

class IDChangeLog;
class UMLAssociation;

/**
 *	This class contains the non-graphical information required for UMLObjects which appear as
 *	moveable widgets on the canvas.
 *	This class inherits from @ref UMLObject which contains most of the information.
 *	It is not instantiated itself, it's just used as a super class for actual model objects.
 *
 *	@short	Non-graphical information for a UMLCanvasObject.
 *	@author Jonathan Riddell
 *	@see	UMLObject
 */

class UMLCanvasObject : public UMLObject {
public:
	/**
	 *	Sets up a UMLCanvasObject.
	 *
	 *	@param	parent	The parent to this Concept.
	 *	@param	name	The name of the Concept.
	 *	@param	id	The unique id of the Concept.
	 */
	UMLCanvasObject(QObject* parent, QString Name, int id);

	/**
	 *	Sets up a UMLCanvasObject.
	 *
	 *	@param	parent		The parent to this Concept.
	 */
	UMLCanvasObject(QObject* parent);

	/**
	 *	Standard deconstructor.
	 */
	virtual ~UMLCanvasObject();

	/**
	 * 		Overloaded '==' operator
	 */
  	virtual bool operator==(UMLCanvasObject& rhs);
  
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
 	 *	Determine if this canvasobject has this association.
 	 *	@param	assoc	The association to check.
 	 */
 	bool hasAssociation(UMLAssociation* assoc);
 
 	/**
 	 *	Remove an association from the CanvasObject.
 	 *
 	 *	@param	o	The association to remove.
 	 */
 	int removeAssociation(UMLObject *o);
 
	/**
	 *	Returns the number of associations for the CanvasObject.
	 *	This is the sum of the aggregations and compositions.
	 *
	 *	@return	The number of associations for the Concept.
	 */
	int associations();

 	/**
	 *	Return the list of associations for the CanvasObject.
	 *
	 *	@return The list of associations for the CanvasObject.
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
	virtual QPtrList<UMLAssociation> getGeneralizations();

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
	 * Returns a name for the new association, operation, template
	 * or attribute appended with a number if the default name is
	 * taken e.g. new_association, new_association_1 etc
	 */
	virtual QString uniqChildName(const UMLObject_Type type);

protected:
	/**
	 *	Initialises key variables of the class.
	 */
	void init();

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

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONCEPT_H
#define CONCEPT_H

#include "umlcanvasobject.h"
#include "umlobjectlist.h"
#include "umlclassifierlist.h"
#include "umloperationlist.h"
#include <qptrlist.h>

class IDChangeLog;
class UMLClassifierListItem;
class UMLStereotype;
class UMLDoc;

/**
 * This is an abstract class which defines the non-graphical information/
 * interface required for a UML Concept (ie a class or interface).
 * This class inherits from @ref UMLCanvasObject which contains most of the
 * information.
 *
 * The @ref UMLDoc class creates instances of this type.  All Concepts will
 * need a unique id.  This will be given by the @ref UMLDoc class.  If you
 * don't leave it up to the @ref UMLDoc class then call the method @ref
 * UMLDoc::getUniqueID to get a unique id.
 *
 * @short Information for a non-graphical Concept/Class.
 * @author Paul Hensgen	<phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLClassifier : public UMLCanvasObject {
	Q_OBJECT
public:
	
	/**
	 * Enumeration identifying the type of classifier.
	 */
	enum ClassifierType { ALL = 0, CLASS, INTERFACE, DATATYPE };

	/**
	 * Sets up a Concept.
	 *
	 * @param parent	The parent to this Concept.
	 * @param name		The name of the Concept.
	 * @param id		The unique id of the Concept.
	 */
	UMLClassifier(UMLDoc * doc, const QString & name = "", int id = -1);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLClassifier();

	/**
	 * Overloaded '==' operator.
	 */
  	bool operator==( UMLClassifier & rhs );
  
	/**
	 * Adds an operation to the Concept.
	 *
	 * @param name		The name of the operation.
	 * @param id		The id of the operation.
	 * @return	Pointer to the UMLOperation created.
	 */
	UMLObject* addOperation(QString name, int id);

	/**
	 * Adds an operation to the classifier, at the given position.
	 * If position is negative or too large, the attribute is added
	 * to the end of the list.
	 *
	 * @param Op		Pointer to the UMLOperation to add.
	 * @param position	Index at which to insert into the list.
	 */
	bool addOperation(UMLOperation* Op, int position = -1);
	

	/**
	 * Adds an already created Operation and checks for operations with
	 * the same name. The Operation must not belong to any other concept.
	 * Used by the clipboard when pasteing.
	 *
	 * @param Op		Pointer to the UMLOperation to add.
	 * @param Log		Pointer to the IDChangeLog.
	 * @return	True if the operation was added successfully.
	 */
	bool addOperation(UMLOperation* Op, IDChangeLog* Log);

	/**
	 * Remove an operation from the Concept.
	 *
	 * @param o		The operation to remove.
	 * @return	Count of the remaining objects after removal.
	 *		Returns -1 if the given operation was not found.
	 */
	int removeOperation(UMLObject *o);

	/**
	 * Add an already created stereotype to the list identified by the
	 * UMLObject_Type.
	 *
	 * @param newStereotype	Pointer to the UMLStereotype to add.
	 * @param list		Object type giving the list on which to add.
	 * @param log		Pointer to the IDChangeLog.
	 * @return	True if the stereotype was successfully added.
	 */
	virtual bool addStereotype(UMLStereotype* newStereotype, UMLObject_Type list, IDChangeLog* log = 0);
	
	/**
	 * Returns the number of operations for the Concept.
	 *
	 * @return	The number of operations for the Concept.
	 */
	int operations();

	/**
	 * Return the list of operations for the Concept.
	 *
	 * @return	The list of operations for the Concept.
	 */
	QPtrList<UMLClassifierListItem>* getOpList();

	/**
	 * Returns the entries in m_pOpsList that are actually operations.
	 *
	 * @return	The list of true operations for the Concept.
	 */
	QPtrList<UMLOperation>* getFilteredOperationsList();

	/**
	 * Returns a name for the new association, operation, template
	 * or attribute appended with a number if the default name is
	 * taken e.g. new_association, new_association_1 etc.
	 * The classes inheriting from UMLClassifier must implement this method.
	 *
	 * @param type		The object type for which to generate a name.
	 * @return	Unique name for the UMLObject_Type given.
	 */
	virtual QString uniqChildName(const UMLObject_Type type) = 0;

	/**
	 * Find a list of attributes, operations, associations or
	 * templates with the given name.
	 *
	 * @param t		The type to find.
	 * @param n		The name of the object to find.
	 * @return	The list of objects found; will be empty if none found.
	 */
	virtual UMLObjectList findChildObject(UMLObject_Type t, QString n);

	/**
	 * Find an attribute, operation, association or template.
	 *
	 * @param id		The id of the object to find.
	 *
	 * @return	The object found.  Will return 0 if none found.
	 */
	virtual UMLObject* findChildObject(int id);

        /**
         * Returns a list of concepts which this concept inherits from.
	 *
	 * @param doc		Pointer to the UMLDoc to use for the lookup.
	 * @param type		The ClassifierType to seek.
         * @return	List of UMLClassifiers we inherit from.
         */
        UMLClassifierList findSuperClassConcepts ( UMLDoc *doc, ClassifierType type = ALL);

        /**
         * Returns a list of concepts which inherit from this concept.
	 *
	 * @param doc		Pointer to the UMLDoc to use for the lookup.
	 * @param type		The ClassifierType to seek.
         * @return	List of UMLClassifiers that inherit from us.
         */
        UMLClassifierList findSubClassConcepts ( UMLDoc *doc, ClassifierType type = ALL);

	/** reimplemented from UMLObject */
	virtual bool acceptAssociationType(Uml::Association_Type);

	//
	// now a number of pure virtual methods..
	//
	
	/**
	 * Creates the XMI element including its operations, attributes and
	 * templates.
	 * This method must be implemented by the inheriting classes.
	 */
	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) = 0;

	/**
	 * Loads the XMI element including its operations, attributes and
	 * templates.
	 * This method must be implemented by the inheriting classes.
	 */
	virtual bool loadFromXMI( QDomElement & element ) = 0;

	/**
	 * Returns true if this classifier represents an interface.
	 * This method must be implemented by the inheriting classes.
	 */
	virtual bool isInterface () = 0;

signals:
	void operationAdded(UMLObject *);
	void operationRemoved(UMLObject*);

protected:

	/**
	 * List of all the operations in this classifier.
	 */
	QPtrList<UMLClassifierListItem> m_OpsList;

private:

	/**
	 * Initializes key variables of the class.
	 */ 
	void init(); // doesnt seem to be any reason for this to be public 

};

#endif // CONCEPT_H 

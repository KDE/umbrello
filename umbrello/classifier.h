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
#include "umlattributelist.h"
#include "umloperationlist.h"
#include "umlclassifierlistitemlist.h"
#include <qptrlist.h>

class IDChangeLog;
class UMLStereotype;

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
	 * @param name		The name of the Concept.
	 * @param id		The unique id of the Concept.
	 */
	UMLClassifier(const QString & name = "", int id = -1);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLClassifier();

	/**
	 * Overloaded '==' operator.
	 */
	bool operator==( UMLClassifier & rhs );

	/**
	 * Copy the internal presentation of this object into the new
	 * object.
	 */
	virtual void copyInto(UMLClassifier *rhs) const;

	/**
	 * Make a clone of this object. This function is abstract
	 * since it is not possible to realise a clone. It has other abstract
	 * functions. Underlying classes must implement the clone functionality.
	 */
	virtual UMLObject* clone() const = 0;

	/**
	 * Adds an operation to the classifier, at the given position.
	 * If position is negative or too large, the attribute is added
	 * to the end of the list.
	 * The Classifier first checks and only adds the Operation if the
	 * signature does not conflict with exising operations
	 *
	 * @param Op		Pointer to the UMLOperation to add.
	 * @param position	Index at which to insert into the list.
	 * @return true if the Operation could be added to the Classifier.
	 */
	bool addOperation(UMLOperation* Op, int position = -1);

	/**
	 * Appends an operation to the classifier.
	 * @see bool addOperation(UMLOperation* Op, int position = -1)
	 * This function is mainly intended for the clipboard.
	 *
	 * @param Op		Pointer to the UMLOperation to add.
	 * @param Log		Pointer to the IDChangeLog.
	 * @return	True if the operation was added successfully.
	 */
	bool addOperation(UMLOperation* Op, IDChangeLog* Log);

	/**
	 * Checks whether an operation is valid based on its signature -
	 * An operation is "valid" if the operation's name and paramter list
	 * are unique in the classifier.
	 *
	 * @param name		Name of the operation to check.
	 * @param opParams	Pointer to the method argument list.
	 * @param exemptOp	Pointer to the exempt method (optional.)
	 * @return	NULL if the signature is valid (ok), else return a pointer
	 *		to the existing UMLOperation that causes the conflict.
	 */
	UMLOperation * checkOperationSignature( QString name,
						UMLAttributeList *opParams,
						UMLOperation *exemptOp = NULL);

	/**
	 * Remove an operation from the Classifier.
	 * The operation is not deleted so the caller is responsible for what
	 * happens to it after this.
	 *
	 * @param op	The operation to remove.
	 * @return	Count of the remaining operations after removal, or
	 *		-1 if the given operation was not found.
	 */
	int removeOperation(UMLOperation *op);

	/**
	 * Take and return an operation from class.
	 * It is the callers responsibility to pass on ownership of
	 * the returned operation (or to delete the operation)
	 *
	 * @param  o operation to take
	 * @return pointer to the operation or null if not found.
	 */
	UMLOperation* takeOperation(UMLOperation* o);

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
	 * Remove a stereotype from the Classifier.
	 * The stereotype is not deleted so the caller is responsible for what
	 * happens to it after this.
	 *
	 * @param stype    The stereotype to remove.
	 * @return      Count of the remaining stereotypes after removal, or
	 *              -1 if the given operation was not found.
	 */
	int removeStereotype (UMLStereotype *stype);

	/**
	 * counts the number of operations in the Classifier.
	 *
	 * @return	The number of operations for the Classifier.
	 */
	int operations();

	/**
	 * counts the number of stereotypes in the Classifier.
	 *
	 * @return	The number of stereotypes for the Classifier.
	 */
	int stereotypes();

	/**
	 * Return a list of operations for the Classifier.
	 * @param includeInherited Includes operations from superclasses.
	 *
	 * @return	The list of operations for the Classifier.
	 */
	UMLClassifierListItemList getOpList(bool includeInherited = false);

	/**
	 * Returns the entries in m_pOpsList that are actually operations.
	 *
	 * @return	The list of true operations for the Concept.
	 */
	UMLOperationList getFilteredOperationsList(bool includeInherited = false);

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
	 * Find the object of the given (non-numeric) auxiliary ID
	 * in m_OpsList.  The auxiliary ID is the ID returned by
	 * UMLObject::getAuxId() and is currently only used for
	 * loading foreign XMI files.
	 *
	 * @param idStr		The ID to seek.
	 * @return	Pointer to the UMLObject found or NULL if not found.
	 */
	UMLObject* findChildObjectByIdStr(QString idStr);

	/**
	 * Returns a list of concepts which this concept inherits from.
	 *
	 * @param type		The ClassifierType to seek.
	 * @return	List of UMLClassifiers we inherit from.
	 */
	UMLClassifierList findSuperClassConcepts(ClassifierType type = ALL);

	/**
	 * Returns a list of concepts which inherit from this concept.
	 *
	 * @param type		The ClassifierType to seek.
	 * @return	List of UMLClassifiers that inherit from us.
	 */
	UMLClassifierList findSubClassConcepts(ClassifierType type = ALL);

	/** reimplemented from UMLObject */
	virtual bool acceptAssociationType(Uml::Association_Type);

	//
	// now a number of pure virtual methods..
	//

	/**
	 * Returns true if this classifier represents an interface.
	 * This method must be implemented by the inheriting classes.
	 */
	virtual bool isInterface () = 0;

	/**
	 * return back whether or not this classfiier has abstract operations in it.
	 */
	bool hasAbstractOps ();

signals:
	/** Signals that a new UMLOperation has been added to the classifer.
	 * The signal is emitted in addition to the generic childObjectAdded()
	 */
	void operationAdded(UMLOperation *);

	/** Signals that a UMLOperation has been removed from the classifer.
	 * The signal is emitted in addition to the generic childObjectRemoved()
	 */
	void operationRemoved(UMLOperation *);

	/** Signals that a new UMLStereotype has been added to the classifer.
	 * The signal is emitted in addition to the generic childObjectAdded()
	 */
	void stereotypeAdded (UMLStereotype *);

	/** Signals that a UMLStereotype has been removed from the classifer.
	 * The signal is emitted in addition to the generic childObjectRemoved()
	 */
	void stereotypeRemoved(UMLStereotype *);

protected:

	/**
	 * List of all the operations in this classifier.
	 */
	UMLClassifierListItemList m_OpsList;

private:

	/**
	 * Initializes key variables of the class.
	 */
	void init();

protected:
	/**
	 * Auxiliary to loadFromXMI:
	 * The loading of operations is implemented here.
	 * Calls loadSpecialized() for any other tag.
	 * Child classes can override the loadSpecialized method
	 * to load its additional tags.
	 */
	virtual bool load(QDomElement& element);

	/**
	 * Auxiliary to load().
	 * The implementation is empty here.
	 * Child classes can override this method to implement
	 * the loading of additional tags.
	 */
	virtual bool loadSpecialized(QDomElement& element);

};

#endif // CONCEPT_H

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

#include "package.h"
#include "umlattributelist.h"
#include "umloperationlist.h"
#include "umlclassifierlistitemlist.h"
#include "classifierlistitem.h"
#include "umltemplatelist.h"
#include "model_utils.h"

class IDChangeLog;

/**
 * This is an abstract class which defines the non-graphical information/
 * interface required for a UML Classifier (ie a class or interface).
 * This class inherits from @ref UMLPackage which allows classifiers
 * to also act as namespaces, i.e. it allows classifiers to nest.
 *
 * The @ref UMLDoc class creates instances of this type.  All Classifiers
 * need a unique id.  This will be given by the @ref UMLDoc class.  If you
 * don't leave it up to the @ref UMLDoc class then call the method @ref
 * UMLDoc::getUniqueID to get a unique id.
 *
 * @short Information for a non-graphical Concept/Class.
 * @author Paul Hensgen	<phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLClassifier : public UMLPackage {
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
	UMLClassifier(const QString & name = "", Uml::IDType id = Uml::id_None);

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
	 * Creates an operation in the current document.
	 * The new operation is initialized with name, id, etc.
	 * If a method with the given profile already exists in the classifier,
	 * no new method is created and the existing operation is returned.
	 * If no name is provided, or if the params are NULL, an Operation
	 * Dialog is shown to ask the user for a name and parameters.
	 * The operation's signature is checked for validity within the parent
	 * classifier.
	 *
	 * @param name		The operation name (will be chosen internally if
	 *			none given.)
	 * @param isExistingOp	Optional pointer to bool. If supplied, the bool is
	 *			set to true if an existing operation is returned.
	 * @param params	Optional list of parameter names and types.
	 *			If supplied, new operation parameters are
	 *			constructed using this list.
	 * @return The new operation, or NULL if the operation could not be
	 *         created because for example, the user canceled the dialog
	 *         or no appropriate name can be found.
	*/
	UMLOperation* createOperation( const QString &name = QString::null,
				       bool *isExistingOp = NULL,
				       Umbrello::NameAndType_List *params = NULL);

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
	 * counts the number of operations in the Classifier.
	 *
	 * @return	The number of operations for the Classifier.
	 */
	int operations();

	/**
	 * Return a list of operations for the Classifier.
	 * @param includeInherited Includes operations from superclasses.
	 *
	 * @return	The list of operations for the Classifier.
	 */
	UMLOperationList getOpList(bool includeInherited = false);

	/**
	 * Creates a template for the concept.
	 *
	 * @return	The UMLTemplate created
	 */
	UMLObject* createTemplate(QString name = QString::null);

	/**
	 * Adds a template to the class if it is not there yet.
	 *
	 * @param name		The name of the template.
	 * @param id		The id of the template.
	 * @return	Pointer to the UMLTemplate object created.
	 */
	UMLTemplate* addTemplate(const QString &name, Uml::IDType id = Uml::id_None);

	/**
	 * Adds an already created template.
	 * The template object must not belong to any other concept.
	 *
	 * @param newTemplate	Pointer to the UMLTemplate object to add.
	 * @param log		Pointer to the IDChangeLog.
	 * @return	True if the template was sucessfully added.
	 */
	bool addTemplate(UMLTemplate* newTemplate, IDChangeLog* log = 0);

	/**
	 * Adds an template to the class.
	 * The template object must not belong to any other class.
	 *
	 * @param Template	Pointer to the UMLTemplate to add.
	 * @param position	The position of the template in the list.
	 *			A value of -1 will add the template at the end.
	 * @return	True if the template was sucessfully added.
	 */
	 //TODO: if the param IDChangeLog from the method above is not being used,
	  // give position a default value of -1 and the method can replace the above one
	bool addTemplate(UMLTemplate* Template, int position);

	/**
	 * Removes a template from the class.
	 *
	 * @param template	The template to remove.
	 * @return	Count of the remaining templates after removal.
	 *		Returns -1 if the given template was not found.
	 */
	int removeTemplate(UMLTemplate* umltemplate);

	/**
	 * Take and return a template parameter from class.
	 * It is the callers responsibility to pass on ownership of
	 * the returned template (or to delete the template)
	 *
	 * @param t		template to take
	 * @return	pointer to the template or null if not found.
	 */
	UMLTemplate* takeTemplate(UMLTemplate* t);

	/**
	 * Seeks the template parameter of the given name.
	 */
	UMLTemplate *findTemplate(QString name);

	/**
	 * Returns the number of templates for the class.
	 *
	 * @return	The number of templates for the class.
	 */
	int templates();

	/**
	 * Returns the templates.
	 * Same as UMLClassifier::getFilteredList(ot_Template) but
	 * return type is a true UMLTemplateList.
	 *
	 * @return	Pointer to the list of true templates for the class.
	 */
	UMLTemplateList getTemplateList();


	/**
	 * Returns the entries in m_List that are of the requested type.
	 *
	 * @return	The list of true operations for the Concept.
	 */
	UMLClassifierListItemList getFilteredList(Uml::Object_Type ot);

	/**
	 * Needs to be called after all UML objects are loaded from file.
	 * Calls the parent resolveRef(), and calls resolveRef() on all
	 * UMLClassifierListItems.
	 * Overrides the method from UMLObject.
	 *
	 * @return 	true for success.
	 */
	virtual bool resolveRef();

	/**
	 * Find a list of attributes, operations, associations or
	 * templates with the given name.
	 *
	 * @param t		The type to find.
	 * @param n		The name of the object to find.
	 * @return	The list of objects found; will be empty if none found.
	 */
	virtual UMLObjectList findChildObject(Uml::Object_Type t, const QString &n);

	/**
	 * Find an attribute, operation, association or template.
	 *
	 * @param id		The id of the object to find.
	 *
	 * @return	The object found.  Will return 0 if none found.
	 */
	virtual UMLObject* findChildObject(Uml::IDType id);

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

	void templateAdded(UMLObject*);
	void templateRemoved(UMLObject*);

protected:

	/**
	 * List of items (operations/attributes/templates) in this classifier.
	 */
	UMLClassifierListItemList m_List;

private:

	/**
	 * Initializes key variables of the class.
	 */
	void init();

protected:

	/**
	 * Auxiliary to saveToXMI of inheriting classes:
	 * Saves template parameters to the given QDomElement.
	 */
	void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

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


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLCLASS_H
#define UMLCLASS_H

#include "classifier.h"

class UMLAttribute;
class UMLTemplate;
class UMLClassifierListItem;

// This class performs the functionality of the old UMLClassifier class

/**
 *	This class contains the non-graphical information required for a UML Class.
 *	This class inherits from @ref UMLClassifier which contains most of the information.
 *	The @ref UMLDoc class creates instances of this type.  All Classes will need a unique
 *	id.  This will be given by the @ref UMLDoc class.  If you don't leave it up to the @ref UMLDoc
 *	class then call the method @ref UMLDoc::getUniqueID to get a unique id.
 *
 *	@short	Information for a non-graphical Concept/Class.
 *	@author Brian Thomas <brian.thomas@gsfc.nasa.gov>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLClass : public UMLClassifier {
Q_OBJECT
public:
	/**
	 *	Sets up a Class.
	 *
	 *	@param	parent	The parent to this Concept.
	 *	@param	name	The name of the Concept.
	 *	@param	id	The unique id of the Concept.
	 */
	UMLClass(QObject* parent, QString Name, int id);

	/**
	 *	Sets up a Concept.
	 *
	 *	@param	parent		The parent to this Concept.
	 */
	UMLClass(QObject* parent);

	/**
	 *	Standard deconstructor.
	 */
	virtual ~UMLClass();

	/**
	 * 		Overloaded '==' operator
	 */
  	bool operator==( UMLClass & rhs );
  
 	/**
  	 *	Adds an attribute to the Concept.
  	 *
  	 *	@param	name	The name of the Attribute.
	 *	@param	id			The id of the Attribute.
	 */
	UMLObject* addAttribute(QString name, int id);

	/**
	 * Adds an already created attribute, the attribute object must not belong to any other
	 *	concept
	 */
	bool addAttribute(UMLAttribute* Att, IDChangeLog* Log = 0);
	
	/** Adds an attribute to the class, at the given position. If position is negative or too large,
	 * the attribute is added to the end of the list */
	 //TODO:  give default value -1 to position (append) - now it conflicts with the method above..
	bool addAttribute(UMLAttribute* Att, int position );

	/**
	 *	Removes an attribute from the Concept.
	 *
	 *	@param	a	The attribute to remove.
	 */
	int removeAttribute(UMLObject *a);

 	/**
  	 *	Adds a template to the Concept.
  	 *
  	 *	@param	name	The name of the template.
	 *	@param	id	The id of the template.
	 */
	UMLObject* addTemplate(QString name, int id);

	/**
	 * Adds an already created template, the template object must not belong to any other
	 *	concept
	 */
	bool addTemplate(UMLTemplate* newTemplate, IDChangeLog* log = 0);

	/**
	 * Adds an template to the class, the template object must not belong to any other
	 * class
	 * @param Template The template to add
	 * @param position The position of the template in the list. A value of -1 will
	 *                 add the template to the end of the list
	 */
	 //TODO: if the param IDChangeLog from the method above is not being used,
	  // give position a default value of -1 and the method can replace the above one
	bool addTemplate(UMLTemplate* Template, int position);

	/**
	 *	Removes a template from the Concept.
	 *
	 *	@param	a	The template to remove.
	 */
	int removeTemplate(UMLTemplate* a);

	/**
	 * Add an already created stereotype to the list given by UMLObject_type list.
	 */
	virtual bool addStereotype(UMLStereotype* newStereotype, UMLObject_Type list, IDChangeLog* log = 0);

	/**
	 *	Returns the number of attributes for the Concept.
	 *
	 *	@return	The number of attributes for the Concept.
	 */
	int attributes();

	/**
	 *	Returns the number of templates for the Concept.
	 *
	 *	@return	The number of templates for the Concept.
	 */
	int templates();

	/**
	 *	Return the list of attributes for the Concept.
	 *
	 *	@return The list of attributes for the Concept.
	 */
	QPtrList<UMLClassifierListItem>* getAttList();

	/**
	 * Returns the entries in m_pAttList that are actually attributes
	 */
	QPtrList<UMLAttribute>* getFilteredAttributeList();

	/**
	 *	Return the list of templates for the Concept.
	 *
	 *	@return The list of templates for the Concept.
	 */
	QPtrList<UMLClassifierListItem>* getTemplateList();

	/**
	 * Returns the entries in m_pTemplatesList that are actually templates
	 */
	QPtrList<UMLTemplate>* getFilteredTemplateList();

	/**
	 *      Find a list of attributes, operations, associations or
	 *      templates with the given name.
	 *
	 *      @param  t       The type to find.
	 *      @param  n       The name of the object to find.
	 *
	 *      @return The object found.  Will return 0 if none found.
	 */
	 virtual QPtrList<UMLObject> findChildObject(UMLObject_Type t, QString n);

	/**
	 *      Find an attribute, operation, association or template.
	 *
	 *      @param  id      The id of the object to find.
	 *
	 *      @return The object found.  Will return 0 if none found.
	 */
	virtual UMLObject* findChildObject(int id);

	/**
	 * Return true if this class has an enumeration stereotype.
	 * Currently the following stereotypes are supported:
	 * <<Enumeration>>  <<enumeration>>  <<CORBAEnum>>
	 */
	bool isEnumeration();

	/**
	 * Returns a name for the new association, operation, template
	 * or attribute appended with a number if the default name is
	 * taken e.g. new_association, new_association_1 etc
	 */
	virtual QString uniqChildName(const UMLObject_Type type);

	/**
	 * Creates the UML:Concept element including it's operations, attributes and templates
	 */
	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the UML:Class element including it's operations, attributes and templates
	 */
	virtual bool loadFromXMI( QDomElement & element );
signals:
	void attributeAdded(UMLObject*);
	void attributeRemoved(UMLObject*);
	void templateAdded(UMLObject*);
	void templateRemoved(UMLObject*);

protected:

	/**
	 *	Initializes key variables of the class.
	 */
	virtual void init(); 

private:

	/**
	 * 	List of all the attributes in this class.
	 */
	QPtrList<UMLClassifierListItem> m_AttsList;

	/**
	 * 	List of all the templates in this class.
	 */
	QPtrList<UMLClassifierListItem> m_TemplateList;
};

#endif // UMLCLASS_H 


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
#include "umlobjectlist.h"
#include "umlattributelist.h"
#include "umltemplatelist.h"
#include "umlclassifierlistitemlist.h"

/**
 * This class contains the non-graphical information required for a UML Class.
 * This class inherits from @ref UMLClassifier which contains most of the
 * information.
 *
 * The @ref UMLDoc class creates instances of this type.  All Classes will
 * need a unique id.  This will be given by the @ref UMLDoc class.
 * If you don't leave it up to the @ref UMLDoc class then call the method
 * @ref UMLDoc::getUniqueID to get a unique id.
 *
 * @short Information for a non-graphical UML class.
 * @author Brian Thomas <brian.thomas@gsfc.nasa.gov>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLClass : public UMLClassifier {
	Q_OBJECT
public:
	/**
	 * Sets up a Class.
	 *
	 * @param name		The name of the class.
	 * @param id		The unique id of the class.
	 */
	UMLClass(const QString & Name = "", int id = -1);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLClass();

	/**
	 * Overloaded '==' operator.
	 */
  	bool operator==( UMLClass & rhs );
  
 	/**
  	 * Adds an attribute to the class.
  	 *
  	 * @param name		The name of the attribute.
	 * @param id		The id of the attribute.
	 * @return	Pointer to the UMLAttribute created.
	 */
	UMLAttribute* addAttribute(QString name, int id);

	/**
	 * Adds an already created attribute.
	 * The attribute object must not belong to any other concept.
  	 *
  	 * @param Att		Pointer to the UMLAttribute.
	 * @param Log		Pointer to the IDChangeLog (optional.)
	 * @param position	Position index for the insertion (optional.)
	 *			If the position is omitted, or if it is
	 *			negative or too large, the attribute is added
	 *			to the end of the list.
	 * @return	True if the attribute was sucessfully added.
	 */
	bool addAttribute(UMLAttribute* Att, IDChangeLog* Log = 0,
			  int position = -1);

	/**
	 * Removes an attribute from the class.
	 *
	 * @param a		The attribute to remove.
	 * @return	Count of the remaining attributes after removal.
	 *		Returns -1 if the given attribute was not found.
	 */
	int removeAttribute(UMLObject *a);

 	/**
  	 * Adds a template to the class.
  	 *
  	 * @param name		The name of the template.
	 * @param id		The id of the template.
	 * @return	Pointer to the UMLTemplate object created.
	 */
	UMLObject* addTemplate(QString name, int id);

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
	 * Add an already created stereotype to the list identified by the
	 * given UMLObject_type.
	 *
	 * @param newStereotype	Pointer to the UMLStereotype to add.
	 * @param list		The object type for the list on which to add.
	 * @param log		Pointer to the IDChangeLog.
	 * @return	True if the newStereotype was successfully added.
	 */
	virtual bool addStereotype(UMLStereotype* newStereotype, UMLObject_Type list, IDChangeLog* log = 0);

	/**
	 * Returns the number of attributes for the class.
	 *
	 * @return	The number of attributes for the class.
	 */
	int attributes();

	/**
	 * Returns the number of templates for the class.
	 *
	 * @return	The number of templates for the class.
	 */
	int templates();

	/**
	 * Return the list of attributes for the class.
	 *
	 * @return	Pointer to the list of attributes for the class.
	 */
	UMLClassifierListItemList* getAttList();

	/**
	 * Returns the entries in m_pAttList that are actually attributes.
	 *
	 * @return	Pointer to the list of true attributes for the class.
	 */
	UMLAttributeList* getFilteredAttributeList();

	/**
	 * Return the list of templates for the class.
	 *
	 * @return	Pointer to the list of templates for the class.
	 */
	UMLClassifierListItemList* getTemplateList();

	/**
	 * Returns the entries in m_pTemplatesList that are actually templates
	 *
	 * @return	Pointer to the list of true templates for the class.
	 */
	UMLTemplateList* getFilteredTemplateList();

	/**
	 * Find a list of attributes, operations, associations or
	 * templates with the given name.
	 *
	 * @param t		The type to find.
	 * @param n		The name of the object to find.
	 * @return	List of objects found.  Will be empty if none found.
	 */
	 virtual UMLObjectList findChildObject(UMLObject_Type t, QString n);

	/**
	 * Find an attribute, operation, association or template.
	 *
	 * @param id		The id of the object to find.
	 * @return	Pointer to the object found; NULL if not found.
	 */
	virtual UMLObject* findChildObject(int id);

	/**
	 * Return true if this class has an enumeration stereotype.
	 * Notice that since Umbrello now has a proper Enum construct,
	 * it is no longer necessary to use stereotypes to produce
	 * enumerations.
	 * Currently the following stereotypes are supported:
	 * <<enum>> <<Enumeration>>  <<enumeration>>  <<CORBAEnum>>
	 *
	 * @return	True if this class has an enumeration stereotype.
	 */
	bool isEnumeration();

	/**
	 * Returns a name for the new association, operation, template
	 * or attribute appended with a number if the default name is
	 * taken e.g. new_association, new_association_1 etc.
	 *
	 * @param type		The object type for which to generate the name.
	 * @return	Unique name string for the type.
	 */
	virtual QString uniqChildName(const UMLObject_Type type);

	/**
	 * Creates the <UML:Class> XMI element including its operations,
	 * attributes and templates.
	 */
	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <UML:Class> XMI element including its operations,
	 * attributes and templates.
	 */
	virtual bool loadFromXMI( QDomElement & element );

	/**
	 * Determines whether this class acts as an interface.
	 *
	 * @return	True if the class is an interface.
	 */
	virtual bool isInterface () { return false; }

signals:
	void attributeAdded(UMLObject*);
	void attributeRemoved(UMLObject*);
	void templateAdded(UMLObject*);
	void templateRemoved(UMLObject*);

private:

	/**
	 * Initializes key variables of the class.
	 */
	void init(); 

	/**
	 * Auxiliary to loadFromXMI.
	 */
	bool load(QDomElement& element);

	/**
	 * List of all the attributes in this class.
	 */
	UMLClassifierListItemList m_AttsList;

	/**
	 * List of all the templates in this class.
	 */
	UMLClassifierListItemList m_TemplateList;
};

#endif // UMLCLASS_H 

 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

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
	UMLClass(const QString & Name = "", Uml::IDType id = Uml::id_None);

	/**
	 * Copy the internal presentation of this object into the new
	 * object.
	 */
	virtual void copyInto(UMLClass *rhs) const;

	/**
	 * Make a clone of this object.
	 */
	virtual UMLObject* clone() const;

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
	 * If an attribute of the given name already exists, then
	 * returns the existing attribute instead of creating a new one.
	 *
	 * @param name		The name of the attribute.
	 * @param id		The id of the attribute (optional.)
	 *                      If not given, and the attribute name
	 *                      does not already exist, then the method
	 *                      will internally assign a new ID.
	 * @return	Pointer to the UMLAttribute created or found.
	 */
	UMLAttribute* addAttribute(const QString &name, Uml::IDType id = Uml::id_None);

	UMLAttribute* addAttribute(const QString &name, UMLObject *type, Uml::Scope scope);

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
	 * Take and return an attribute from class.
	 * It is the callers responsibility to pass on ownership of
	 * the returned attribute (or to delete the attribute)
	 *
	 * @param  a		attribute to take
	 * @return	pointer to the attribute or null if not found.
	 */
	UMLAttribute* takeAttribute(UMLAttribute* a);

	/**
	 * Returns the number of attributes for the class.
	 *
	 * @return	The number of attributes for the class.
	 */
	int attributes();

	/**
	 * Returns the attributes.
	 * Same as UMLClassifier::getFilteredList(ot_Attribute) but
	 * return type is a true UMLAttributeList.
	 *
	 * @return	List of true attributes for the class.
	 */
	UMLAttributeList getAttributeList();

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
	 * Creates the <UML:Class> XMI element including its operations,
	 * attributes and templates.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Determines whether this class acts as an interface.
	 *
	 * @return	True if the class is an interface.
	 */
	virtual bool isInterface () { return false; }

signals:
	void attributeAdded(UMLObject*);
	void attributeRemoved(UMLObject*);

protected:
	/**
	 * Called by UMLClassifier::load().
	 * We read the <UML:Attribute>s here.
	 */
	bool loadSpecialized(QDomElement& element);

private:

	/**
	 * Initializes key variables of the class.
	 */
	void init();

};

#endif // UMLCLASS_H


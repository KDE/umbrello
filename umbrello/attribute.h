/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "classifierlistitem.h"
#include "umlnamespace.h"

/**
 * This class is used to set up information for an attribute.  This is like
 * a programming attribute.  It has a type, name, scope and initial value.
 *
 * @short Sets up attribute information.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLAttribute : public UMLClassifierListItem {
public:
	/**
	 * Sets up an attribute.
	 *
	 * @param parent	The parent of this UMLAttribute.
	 * @param name		The name of this UMLAttribute.
	 * @param id		The unique id given to this UMLAttribute.
	 * @param s		The scope of the UMLAttribute.
	 * @param type		The type of this UMLAttribute.
	 * @param iv		The initial value of the attribute.
	 */
	UMLAttribute(const UMLObject *parent, QString name, int id,
		     Uml::Scope s = Uml::Private, QString type = "int", QString iv = 0);

	/**
	 * Sets up an attribute.
	 *
	 * @param parent	The parent of this UMLAttribute.
	 */
	UMLAttribute(const UMLObject *parent);

	/**
	 * Overloaded '==' operator
	 */
	bool operator==( UMLAttribute &rhs);

	/**
	 * destructor.
	 */
	virtual ~UMLAttribute();

	/**
	 * Copy the internal presentation of this object into the UMLAttribute
	 * object.
	 */
	virtual void copyInto(UMLAttribute *rhs) const;

	/**
	 * Make a clone of the UMLAttribute.
	 */
	virtual UMLObject* clone() const;

	/**
	 * Returns The initial value of the UMLAttribute.
	 *
	 * @return	The inital value of the Atrtibute.
	 */
	QString getInitialValue();

	/**
	 * Sets the initial value of the UMLAttribute.
	 *
	 * @param iv		The inital value of the UMLAttribute.
	 */
	void setInitialValue(QString iv);

	/**
	 * Returns a string representation of the UMLAttribute.
	 *
	 * @param sig		If true will show the attribute type and
	 *			inital value.
	 * @return	Returns a string representation of the UMLAttribute.
	 */
	QString toString(Uml::Signature_Type sig);

	/**
	 * This needs to be called after all UML objects have been loaded.
	 * It resolves the xmi.id of the UML:Attribute "type".
	 * This needs to be done after all classifiers are loaded because
	 * the xmi.id might be a "forward reference", i.e. it may identify
	 * a classifier which was not yet loaded at that point.
	 */
	bool resolveType();

	/**
	 * Creates the <UML:Attribute> XMI element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Display the properties configuration dialogue for the attribute.
	 */
	bool showPropertiesDialogue(QWidget* parent);

	void setParmKind (Uml::Parameter_Kind pk);
	Uml::Parameter_Kind getParmKind () const;

protected:
	/**
	 * Loads the <UML:Attribute> XMI element.
	 */
	bool load( QDomElement & element );

private:
	QString m_InitialValue; ///< text for the attribute's initial value.
	Uml::Parameter_Kind m_ParmKind;
};

#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STEREOTYPE_H
#define STEREOTYPE_H

#include "classifierlistitem.h"

/**
 * This class is used to set up information for a stereotype.
 * Stereotypes are used essentially as comments to class together
 * attributes and operations etc.
 *
 * @short Sets up stereotype information.
 * @author Jonathan Riddell
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLStereotype : public UMLClassifierListItem {
public:
	/**
	 * Sets up a stereotype.
	 *
	 * @param parent	The parent of this UMLStereotype.
	 * @param name		The name of this UMLStereotype.
	 * @param id		The unique id given to this UMLStereotype.
	 * @param listType	The list which this stereotype is part of
	 *			(attribute, operation etc)
	 */
	UMLStereotype(UMLObject* parent, QString name, int id, UMLObject_Type listType);

	/**
	 * Sets up a stereotype.
	 *
	 * @param parent	The parent of this UMLStereotype.
	 */
	UMLStereotype(UMLObject* parent);

	/**
	 * Overloaded '==' operator
	 */
	bool operator==(UMLStereotype &rhs);

	/**
	 * destructor
	 */
	virtual ~UMLStereotype();

	/**
	 * Copy the internal presentation of this object into the new
	 * object.
	 */
	virtual void copyInto(UMLStereotype *rhs) const;

	/**
	 * Make a clone of this object.
	 */
	virtual UMLStereotype* clone() const;

	/**
	 * Returns a string representation of the UMLStereotype.
	 * @param sig	If true will show the stereotype type and inital value.
	 *
	 * @return	Returns a string representation of the UMLStereotype.
	 */
	QString toString(Signature_Type sig);

	/**
	 * Saves to the <UML:StereoType> XMI element.
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads a <UML:StereoType> XMI element.
	 */
	bool loadFromXMI(QDomElement& element);

	/**
	 * Display the properties configuration dialogue for the stereotype
	 * (just a line edit).
	 */
	bool showPropertiesDialogue(QWidget* parent);

	/**
	 * Return the stereotype name surrounded by double angle brackets.
	 */
	virtual QString getShortName();
private:
	/**
	 * The list this stereotype is part of (operation, attribute etc)
	 */
	UMLObject_Type m_listType;
};

#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENUMLITERAL_H
#define ENUMLITERAL_H

#include "classifierlistitem.h"

/**
 * This class is used to set up information for an enum literal.  Enum
 * literals are the values that enums can be set to.
 *
 * @short Sets up attribute information.
 * @author Paul Hensgen <phensgen@techie.com>
 * @see UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLEnumLiteral : public UMLClassifierListItem {
public:
	/**
	 * Sets up an enum literal.
	 *
	 * @param parent	The parent of this UMLEnumLiteral.
	 * @param name		The name of this UMLEnumLiteral.
	 * @param id		The unique id given to this UMLEnumLiteral.
	 */
	UMLEnumLiteral(UMLObject* parent, QString name, int id);

	/**
	 * Sets up an enum literal.
	 *
	 * @param parent	The parent of this UMLEnumLiteral.
	 */
	UMLEnumLiteral(UMLObject* parent);

	/**
	 * Overloaded '==' operator
	 */
	bool operator==(UMLEnumLiteral &rhs);

	/**
	 * destructor
	 */
	virtual ~UMLEnumLiteral();

	/**
	 * Returns the type of the UMLEnumLiteral.
	 *
	 * @return	The type of the UMLEnumLiteral.
	 */
	QString getTypeName();

	/**
	 * Returns a string representation of the UMLEnumLiteral.
	 *
	 * @param sig		If true will show the enum literal type and
	 *			inital value.
	 * @return	Returns a string representation of the UMLEnumLiteral.
	 */
	QString toString(Signature_Type sig = st_NoSig);

	/**
	 * Creates the <UML:EnumLiteral> XMI element.
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads the <UML:EnumLiteral> XMI element.
	 */
	bool loadFromXMI(QDomElement& element);

	/**
	 * Display the properties configuration dialogue for the enum literal.
	 */
	bool showPropertiesDialogue(QWidget* parent);

private:

};

#endif

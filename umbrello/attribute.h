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
#include "classifier.h"

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
	 *			CHECK: In contrast to UMLOperation, this is
	 *			currently a UMLDoc. Perhaps this should be
	 *			changed. For example, the UMLAttribute could
	 *			be used in two ways:
	 *			1. Child of UMLClass
	 *			2. Child of UMLOperation (for representing
	 *			   method parameters)
	 * @param name		The name of this UMLAttribute.
	 * @param id		The unique id given to this UMLAttribute.
	 * @param type		The type of this UMLAttribute.
	 * @param s		The scope of the UMLAttribute.
	 * @param iv		The initial value of the attribute.
	 */
	UMLAttribute(UMLDoc * parent, QString name, int id, QString type = "int",
		     Scope s = Private, QString iv = 0);

	/**
	 * Sets up an attribute.
	 *
	 * @param parent	The parent of this UMLAttribute.
	 */
	UMLAttribute(UMLDoc * parent);

	/**
	 * Overloaded '==' operator
	 */
	bool operator==( UMLAttribute &rhs);

	/**
	 * Standard deconstructor.
	 */
	~UMLAttribute();

	/**
	 * Returns the type of the UMLAttribute.
	 *
	 * @return	The type of the UMLAttribute.
	 */
	QString getTypeName();

	/**
	 * Returns The initial value of the UMLAttribute.
	 *
	 * @return	The inital value of the Atrtibute.
	 */
	QString getInitialValue();

       /**
         * Sets the type of the UMLAttribute.
         *
         * @param type		The type of the UMLAttribute.
         */
        void setTypeName(QString type);

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
	QString toString(Signature_Type sig);

	/**
	 * Creates the <UML:Attribute> XMI element.
	 */
	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Loads the <UML:Attribute> XMI element.
	 */
	bool loadFromXMI( QDomElement & element );

	/**
	 * Display the properties configuration dialogue for the attribute.
	 */
	bool showPropertiesDialogue(QWidget* parent);

private:
	QString m_TypeName;     ///< text for the attribute type.
	QString m_InitialValue; ///< text for the attribute's initial value.

};

#endif

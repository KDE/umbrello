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

#include "umlobject.h"

/**
 *	This class is used to set up information for an atrtibute.  This is like
 *	a programming attribute.  It has a type, name, scope and initial value.
 *
 *	@short Sets up attribute information.
 *	@author Paul Hensgen <phensgen@techie.com>
 *	@version 1.0
 *	@see	UMLObject
 */

class UMLAttribute : public UMLObject {
public:
	/**
	 *	Sets up an attribute.
	 *
	 *	@param	parent		The parent of this UMLAttribute.
	 *	@param	name		The name of this UMLAttribute.
	 *	@param	id				The unique id given to this UMLAttribute.
	 *	@param	type		The type of this UMLAttribute.
	 *	@param	s				The scope of the UMLAttribute.
	 *	@param	iv				The initial value of the attribute.
	 */
	UMLAttribute(QObject * parent, QString name, int id, QString type = "int", Scope s = Public, QString iv = 0);

	/**
	 *	Sets up an attribute.
	 *
	 *	@param	parent		The parent of this UMLAttribute.
	 */
	UMLAttribute(QObject * parent);

	/**
	 * 	Overloaded '==' operator
	 */
	bool operator==( UMLAttribute &rhs);

	/**
	 *	Standard deconstructor.
	 */
	~UMLAttribute();

	/**
	 *	Returns the type of the UMLAttribute.
	 *
	 *	@return The type of the UMLAttribute.
	 */
	QString getTypeName();

	/**
	 *	Returns The initial value of the UMLAttribute.
	 *
	 *	@return	The inital value of the Atrtibute.
	 */
	QString getInitialValue();

	/**
	 *	Sets the type of the UMLAttribute.
	 *
	 *	@param	type	The type of the UMLAttribute.
	 */
	void setTypeName(QString type) {
		m_TypeName = type;
	}

	/**
	 *	Sets the initial value of the UMLAttribute.
	 *
	 *	@param	iv	The inital value of the UMLAttribute.
	 */
	void setInitialValue(QString iv) {
		m_InitialValue = iv;
	}

	/**
	 * Use to save or load this classes information
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 *                       load the information.
	 *	@param	fileversion	the version of the serialize format
	 *
	 *	@return					Returns the success of the operation.
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	 *    Returns the amount of bytes needed to serialize an instance object to the clipboard
	 */
	virtual long getClipSizeOf();

	/**
	 *	Returns a string representation of the UMLAttribute.
	 *	@param	sig	If true will show the attribute type and inital value.
	 *
	 *	@return	Returns a string representation of the UMLAttribute.
	 */
	QString toString(Signature_Type sig);

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & element );
private:
	QString m_TypeName, m_InitialValue;
};

#endif

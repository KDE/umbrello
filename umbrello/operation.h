/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OPERATION_H
#define OPERATION_H

#include <qptrlist.h>
#include "umlobject.h"

// forward declarations
class UMLAttribute;

/**
 *	This class is used to set up information for an operation.  This is like
 *	a programming operation.  It has a return type, scope, name and paramters.
 *
 *	@short	Holds the information to represent an operation.
 *	@author Paul Hensgen	<phensgen@techie.com>
 *	@version	1.0
 *	@see	UMLObject
 */

class UMLOperation : public UMLObject {
	Q_OBJECT
public:
	/**
	 *	Constructs an UMLOperation.
	 *
	 *	@param	parent		The parent to this operation.
	 *	@param	name		The name of the operation.
	 *	@param	id				The id of the operation.
	 *	@param	s				The scope of the operation.
	 *	@param	rt				The return type of the operation.
	 */
	UMLOperation(QObject * parent, QString Name, int id, Scope s = Public, QString rt = "void");

	/**
	 *	Constructs an UMLOperation.
	 *
	 *	@param	parent		The parent to this operation.
	 */
	UMLOperation(QObject * parent);

	/**
	 * 		Overloaded '==' operator
	 */
	bool operator==( UMLOperation & rhs );

	/**
	 *	Standard deconstructor.
	 */
	~UMLOperation();

	/**
	 *	Sets the return type of the operation.
	 *
	 *	@param	rt	The return type to set the operation to.
	 */
	void setReturnType(QString rt) {
		m_ReturnType = rt;
	}

	/**
	 *	Get the return type of the operation.
	 *
	 *	@return	The return type.
	 */
	QString getReturnType() {
		return m_ReturnType;
	}

	/**
	 *	Add a parameter to the operation.
	 *
	 *	@param	type		The type of the parameter.
	 *	@param	name		The name of the parameter.
	 *	@param	initialValue	The initial value of the parameter.
	 */
	void addParm(QString type, QString name, QString initialValue, QString doc);

	/**
	 *	Remove a parameter from the operation.
	 *
	 *	@param	a		The paramter to remove.
	 */
	void removeParm(UMLAttribute *a);

	/**
	 *	Returns a list of parameters.
	 *
	 *	@return A list of the paramters in the operation.
	 */
	QPtrList<UMLAttribute>* getParmList() {
		return &m_List;
	}

	/**
	 *	Finds a parameter of the operation.
	 *
	 *	@param	name	The parameter name to search for.
	 *	@return	The found parameter, 0 if not found.
	 */
	UMLAttribute * findParm(QString name);

	/**
	 *	Returns a string representation of the operation.
	 *
	 *	@param	sig	What type of operation string to show.
	 *	@return	The string representation of the operation.
	 */
	QString toString(Signature_Type sig = st_NoSig);

	int getUniqueID() {
		return m_nUniqueID;
	}
	/**
	 *	Add a parameter to the operation.
	 *
	 *	@param	parameter The paramter to add
	 *	@param	position The position in the parameter list. If position = -1 the parameter
	 *              will be appended to the list
	 */
	void addParm(UMLAttribute *parameter, int position = -1);

	/**
	 *  Returns an unused parameter name for a new parameter
	 */
	QString getUniqueParameterName();

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & element );

private:
	QString m_ReturnType;
	QPtrList<UMLAttribute> m_List;
	int m_nUniqueID;
};

#endif

/***************************************************************************
 *									   *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
 *									   *
 ***************************************************************************/

#ifndef ASSOCIATION_H
#define ASSOCIATION_H

#include "umlnamespace.h"
#include "umlobject.h"

class UMLDoc;


/**
 *	This class contains the non-graphic representation of an association.
 *	An association can be an aggregation or composition.
 *
 *	@short Sets up association information.
 *	@author Oliver Kellogg <okellogg@users.sourceforge.net>
 *	@version 1.0
 *	@see	UMLObject
 */

class UMLAssociation : public UMLObject {
public:
	/**
	 *	Sets up an association.
	 *
	 *	@param	parent		The parent of this UMLAssociation.
	 *	@param	name		The name of this UMLAssociation.
	 *	@param	id		The unique id given to this UMLAssociation.
	 *	@param	assocType	The association type.
	 *	@param	roleA_id	ID of the UMLObject of role A.
	 *	@param	roleB_id	ID of the UMLObject of role B.
	 *	@param	multiA		The multiplicity of the role A.
	 *	@param	multiB		The multiplicity of the role B.
	 *	@param	nameA		The name of the role A (NYI.)
	 *	@param	nameB		The name of the role B (NYI.)
	 */
	UMLAssociation(UMLDoc* parent, QString name, int id,
		       Uml::Association_Type assocType,
		       int roleA_id, int roleB_id,
		       QString multiA = "", QString multiB = "",
		       QString nameA = "", QString nameB = "")
	: UMLObject((UMLObject*)parent, name, id) {
		m_AssocType = assocType;
		m_RoleA = roleA_id;
		m_RoleB = roleB_id;
		m_MultiA = multiA;
		m_MultiB = multiB;
		m_NameA = nameA;
		m_NameB = nameB;
		m_BaseType = ot_Association;
	}

	/**
	 *	Sets up an association.
	 *
	 *	@param	parent	The parent of this UMLAssociation.
	 */
	UMLAssociation(QObject * parent) : UMLObject(parent) {
		m_AssocType = Uml::at_Unknown;
		m_RoleA = -1;
		m_RoleB = -1;
		m_MultiA = "";
		m_MultiB = "";
		m_NameA = "";
		m_NameB = "";
		m_BaseType = ot_Association;
	}

	/**
	 * 	Overloaded '==' operator
	 */
	bool operator==(UMLAssociation &rhs) {
		if( this == &rhs )
			return true;
		return( UMLObject::operator==( rhs ) &&
			m_AssocType == rhs.m_AssocType &&
			m_RoleA == rhs.m_RoleA &&
			m_RoleB == rhs.m_RoleB &&
			m_MultiA == rhs.m_MultiA &&
			m_MultiB == rhs.m_MultiB &&
			m_NameA == rhs.m_NameA &&
			m_NameB == rhs.m_NameB );
	}

	/**
	 *	Standard deconstructor.
	 */
	~UMLAssociation() {}

	/**
	 *	Converts a Uml::Association_Type to its string representation.
	 *
	 *	@param		The assocType.
	 *	@return		The string representation of the assocType.
	 */
	QString toString (Uml::Association_Type atype);

	/**
	 *	Converts an association type string to the corresponding
	 *	Uml::Association_Type.
	 *
	 *	@param		The string representation of the assocType.
	 *	@return		The assocType.
	 */
	Uml::Association_Type toAssocType (QString atype);

	/**
	 *	Returns the assocType of the UMLAssociation.
	 *
	 *	@return		The assocType of the UMLAssociation.
	 */
	Uml::Association_Type getAssocType() const { return m_AssocType; }

	/**
	 *	Returns the ID of the UMLObject assigned to the role A.
	 *
	 *	@return		Pointer to the UMLObject of role A.
	 */
	int getRoleA() const { return m_RoleA; }

	/**
	 *	Returns the ID of the UMLObject assigned to the role B.
	 *
	 *	@return		Pointer to the UMLObject of role B.
	 */
	int getRoleB() const { return m_RoleB; }

	/**
	 *	Returns the multiplicity assigned to the role A.
	 *
	 *	@return		The multiplicity assigned to the role A.
	 */
	QString getMultiA() const { return m_MultiA; }

	/**
	 *	Returns the multiplicity assigned to the role B.
	 *
	 *	@return		The multiplicity assigned to the role B.
	 */
	QString getMultiB() const { return m_MultiB; }

	/**
	 *	Returns the name assigned to the role A.
	 *
	 *	@return		The name assigned to the role A.
	 */
	QString getNameA() const { return m_NameA; }

	/**
	 *	Returns the name assigned to the role B.
	 *
	 *	@return		The name assigned to the role B.
	 */
	QString getNameB() const { return m_NameB; }

	/**
	 *	Sets the assocType of the UMLAssociation.
	 *
	 *	@param	assocType	The assocType of the UMLAssociation.
	 */
	void setAssocType(Uml::Association_Type assocType) {
		m_AssocType = assocType;
	}

	/**
	 *	Sets the ID of the UMLObject playing role A in the association.
	 *
	 *	@param	multiA	Pointer to the UMLObject of role A.
	 */
	void setRoleA(int roleA) {
		m_RoleA = roleA;
	}

	/**
	 *	Sets the ID of the UMLObject playing role B in the association.
	 *
	 *	@param	multiB	Pointer to the UMLObject of role B.
	 */
	void setRoleB(int roleB) {
		m_RoleB = roleB;
	}

	/**
	 *	Sets the multiplicity of role A of the UMLAssociation.
	 *
	 *	@param	multiA	The multiplicity of role A of the UMLAssociation.
	 */
	void setMultiA(QString multiA) {
		m_MultiA = multiA;
	}

	/**
	 *	Sets the multiplicity of role B of the UMLAssociation.
	 *
	 *	@param	multiB	The multiplicity of role B of the UMLAssociation.
	 */
	void setMultiB(QString multiB) {
		m_MultiB = multiB;
	}

	/**
	 *	Sets the name of role A of the UMLAssociation.
	 *
	 *	@param	nameA	The name of role A of the UMLAssociation.
	 */
	void setNameA(QString nameA) {
		m_NameA = nameA;
	}

	/**
	 *	Sets the name of role B of the UMLAssociation.
	 *
	 *	@param	nameB	The name of role B of the UMLAssociation.
	 */
	void setNameB(QString nameB) {
		m_NameB = nameB;
	}

	/**
	 * Use to save or load this classes information
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 *			load the information.
	 *	@param	fileversion	the version of the serialize format
	 *
	 *	@return			Returns the success of the operation.
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	 *    Returns the amount of bytes needed to serialize an instance object to the clipboard
	 */
	virtual long getClipSizeOf();

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & element );

private:

	/**
	 * If the type Uml::Association_Type is changed then also the following
	 * must be changed accordingly:
	 * atypeFirst, atypeLast, assocTypeStr[], toAssocType(), toString().
	 * The ordering within assocTypeStr must match the enumeration
	 * order of Uml::Association_Type.
	 */
	static const Uml::Association_Type atypeFirst = Uml::at_Generalization;
	static const Uml::Association_Type atypeLast = Uml::at_Activity;
	static const unsigned nAssocTypes = (unsigned)atypeLast - (unsigned)atypeFirst + 1;
	static const QString assocTypeStr[nAssocTypes];

	Uml::Association_Type m_AssocType;
	int m_RoleA;
	int m_RoleB;
	QString m_MultiA, m_MultiB;
	QString m_NameA, m_NameB;
};

#endif

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
	friend class AssociationWidget;
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
	 *	@param	roleNameA	The name of the role A (NYI.)
	 *	@param	roleNameB	The name of the role B (NYI.)
	 *	@param	multiA		The multiplicity of the role A.
	 *	@param	multiB		The multiplicity of the role B.
	 *	@param	visibilityA	The visibility of role A 
	 *	@param	visibilityB	The visibility of role B 
	 *	@param	changeA		The changeability of role A 
	 *	@param	changeB		The changeability of role B 
	 */
	UMLAssociation(UMLDoc* parent, QString name, int id,
		       Uml::Association_Type assocType,
		       int roleA_id, int roleB_id,
		       QString roleNameA = "", QString roleNameB = "", 
		       QString multiA = "", QString multiB = "",
		       Scope visibilityA = Public, Scope visibilityB = Public, 
		       Changeability_Type changeA = chg_Changeable, 
		       Changeability_Type changeB = chg_Changeable 
		       )
	: UMLObject((UMLObject*)parent, name, id) {

		init();

		m_AssocType = assocType;
		m_Name = name;
		m_RoleAId = roleA_id;
		m_RoleBId = roleB_id;
		m_MultiA = multiA;
		m_MultiB = multiB;
		m_RoleNameA = roleNameA;
		m_RoleNameB = roleNameB;
		m_VisibilityA = visibilityA;
		m_VisibilityB = visibilityB;
		m_ChangeabilityA = changeA;
		m_ChangeabilityB = changeB;

	}

	/**
	 *	Sets up an association.
	 *
	 *	@param	parent	The parent of this UMLAssociation.
	 */
	UMLAssociation(QObject * parent) : UMLObject(parent) {
		init();
	}

	/**
	 * 	Overloaded '==' operator
	 */
	bool operator==(UMLAssociation &rhs) {
		if( this == &rhs )
			return true;
		return( UMLObject::operator==( rhs ) &&
			m_AssocType == rhs.m_AssocType &&
			m_RoleAId == rhs.m_RoleAId &&
			m_RoleBId == rhs.m_RoleBId &&
			m_MultiA == rhs.m_MultiA &&
			m_MultiB == rhs.m_MultiB &&
			m_RoleNameA == rhs.m_RoleNameA &&
			m_RoleNameB == rhs.m_RoleNameB );
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
	int getRoleAId() const { return m_RoleAId; }

	/**
	 *	Returns the ID of the UMLObject assigned to the role B.
	 *
	 *	@return		Pointer to the UMLObject of role B.
	 */
	int getRoleBId() const { return m_RoleBId; }

	/**
	 *	Returns the Changeablity of role A.
	 *
	 *	@return		Changeability_Type of roleA.
	 */
	Changeability_Type getChangeabilityA() const { return m_ChangeabilityA; }

	/**
	 *	Returns the Changeablity of role B.
	 *
	 *	@return		Changeability_Type of roleB.
	 */

	Changeability_Type getChangeabilityB() const { return m_ChangeabilityB; }
	/**
	 *	Returns the Changeablity of role B.
	 *
	 *	@return		Changeability_Type of roleB.
	 */

	/**
	 *	Returns the Visibility of role A.
	 *
	 *	@return		Scope of roleA.
	 */
	Scope getVisibilityA() const { return m_VisibilityA; }

	/**
	 *	Returns the Visibility of role B.
	 *	
	 *	@return		Scope of roleB.
	 */
	Scope getVisibilityB() const { return m_VisibilityB; }

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
	QString getRoleNameA() const { return m_RoleNameA; }

	/**
	 *	Returns the name assigned to the role B.
	 *
	 *	@return		The name assigned to the role B.
	 */
	QString getRoleNameB() const { return m_RoleNameB; }

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
	void setRoleAId(int roleA) {
		m_RoleAId = roleA;
	}

	/**
	 *	Sets the ID of the UMLObject playing role B in the association.
	 *
	 *	@param	multiB	Pointer to the UMLObject of role B.
	 */
	void setRoleBId(int roleB) {
		m_RoleBId = roleB;
	}

	/**
	 *	Sets the visibility of role A of the UMLAssociation.
	 *
	 *	@param	value 	Scope of role A visibility.
	 */
	void setVisibilityA (Scope value) { m_VisibilityA = value; }

	/**
	 *	Sets the visibility of role B of the UMLAssociation.
	 *
	 *	@param	value 	Scope of role A visibility.
	 */
	void setVisibilityB (Scope value) { m_VisibilityB = value; }

	/**
	 *	Sets the changeability of role A of the UMLAssociation.
	 *
	 *	@param	value 	Changeability_Type of role A changeability.
	 */
	void setChangeabilityA (Changeability_Type value) { m_ChangeabilityA = value; }

	/**
	 *	Sets the changeability of role B of the UMLAssociation.
	 *
	 *	@param	value 	Changeability_Type of role B changeability.
	 */

	void setChangeabilityB (Changeability_Type value) { m_ChangeabilityB = value; }
		 
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
	 *	@param	roleNameA	The name of role A of the UMLAssociation.
	 */
	void setRoleNameA(QString roleNameA) {
		m_RoleNameA = roleNameA;
	}

	/**
	 *	Sets the name of role B of the UMLAssociation.
	 *
	 *	@param	roleNameB	The name of role B of the UMLAssociation.
	 */
	void setRoleNameB(QString roleNameB) {
		m_RoleNameB = roleNameB;
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

	/**
         * Convert Changeability_Type value into QString representation. 
	 */
	static QString ChangeabilityToString (Uml::Changeability_Type type)
	{
        	switch (type) {
               		case Uml::chg_Frozen:
                        	return "frozen";
                        	break;
                	case Uml::chg_AddOnly:
                        	return "addOnly";
                        	break;
                	case Uml::chg_Changeable:
                	default:
                        	return "changeable";
                        	break;
	        }
	}

        /**
         * Convert Scope value into QString representation. 
         */
	static QString ScopeToString (Uml::Scope scope)
	{
        	switch (scope) {
                	case Uml::Protected:
                        	return "#";
                        	break;
                	case Uml::Private:
                        	return "-";
                        	break;
                	case Uml::Public:
                	default:
                        	return "+";
                        	break;
        	}
	}


protected:

	/** do some initialization at construction time */
	void init () {

		m_AssocType = Uml::at_Unknown;
		m_RoleAId = -1;
		m_RoleBId = -1;
		m_MultiA = "";
		m_MultiB = "";
		m_RoleNameA = "";
		m_RoleNameB = "";
		m_VisibilityA = Public;
		m_VisibilityB = Public;
		m_ChangeabilityA = chg_Changeable; 
		m_ChangeabilityB = chg_Changeable; 
		m_BaseType = ot_Association;
	}

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
	int m_RoleAId;
	int m_RoleBId;
	QString m_Name;
	QString m_RoleNameA, m_RoleNameB;
	QString m_MultiA, m_MultiB;
	Scope m_VisibilityA, m_VisibilityB;
	Changeability_Type m_ChangeabilityA, m_ChangeabilityB;

};

#endif

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
class UMLRole;

using namespace Uml;

/**
 * This class contains the non-graphic representation of an association.
 * An association can be a generalization, aggregation, or composition.
 *
 * @short Sets up association information.
 * @author Oliver Kellogg <okellogg@users.sourceforge.net>
 * @see	UMLObject
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLAssociation : public UMLObject {
	Q_OBJECT
public:
	/**
	 * Sets up an association.
	 *
	 * @param parent	The parent of this UMLAssociation.
	 */
	UMLAssociation(UMLDoc* parent, Association_Type type, UMLObject *roleA = 0, UMLObject *roleB = 0);

	/**
	 * Overloaded '==' operator
	 */
	bool operator==(UMLAssociation &rhs);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLAssociation();

	/**
	 * Returns a String representation of this UMLAssociation.
	 */
	QString toString( ) const;

	/**
	 * Converts a Uml::Association_Type to its string representation.
	 *
	 * @param atype		The Association_Type enum value to convert.
	 * @return	The string representation of the Association_Type.
	 */
	static QString typeAsString(Uml::Association_Type atype);

	/**
	 * Converts an association type string to the corresponding
	 * Uml::Association_Type.
	 *
	 * @param atype		The string representation of the Association_Type.
	 * @return	The corresponding Association_Type enum value.
	 *		This is set to at_Unknown if the string is not a valid
	 *		Association_Type.
	 */
	static Uml::Association_Type toAssocType(QString atype);

	/**
	 * Returns true if the given Association_Type has a representation as a
	 * UMLAssociation.
	 */
	static bool assocTypeHasUMLRepresentation(Uml::Association_Type atype);

	/**
	 * Returns the Association_Type of the UMLAssociation.
	 *
	 * @return	The Association_Type of the UMLAssociation.
	 */
	Uml::Association_Type getAssocType() const;

	/**
	 * Returns the UMLObject assigned to the role A.
	 *
	 * @return	Pointer to the UMLObject in role A.
	 */
	UMLObject* getObjectA();

	/**
	 * Returns the UMLObject assigned to the role B.
	 *
	 * @return	Pointer to the UMLObject in role B.
	 */
	UMLObject* getObjectB();

	/**
	 * Returns the ID of the UMLObject assigned to the role A.
	 *
	 * @return	ID of the UMLObject of role A.
	 */
	int getRoleAId() const;

	/**
	 * Returns the ID of the UMLObject assigned to the role B.
	 *
	 * @return	ID of the UMLObject of role B.
	 */
	int getRoleBId() const;

	/**
	 * Returns the Changeablity of role A.
	 *
	 * @return	Changeability_Type of roleA.
	 */
	Changeability_Type getChangeabilityA() const;

	/**
	 * Returns the Changeablity of role B.
	 *
	 * @return	Changeability_Type of roleB.
	 */

	Changeability_Type getChangeabilityB() const;

	/**
	 * Returns the Visibility of role A.
	 *
	 * @return	Scope of roleA.
	 */
	Scope getVisibilityA() const;

	/**
	 * Returns the Visibility of role B.
	 *
	 * @return	Scope of roleB.
	 */
	Scope getVisibilityB() const;

	/**
	 * Returns the multiplicity assigned to the role A.
	 *
	 * @return	The multiplicity assigned to the role A.
	 */
	QString getMultiA() const;

	/**
	 * Returns the multiplicity assigned to the role B.
	 *
	 * @return	The multiplicity assigned to the role B.
	 */
	QString getMultiB() const;

	/**
	 * Returns the name assigned to the role A.
	 *
	 * @return	The name assigned to the role A.
	 */
	QString getRoleNameA() const;

	/**
	 * Returns the name assigned to the role B.
	 *
	 * @return	The name assigned to the role B.
	 */
	QString getRoleNameB() const;

	/**
	 * Returns the documentation assigned to the role A.
	 *
	 * @return	Documentation text of role A.
	 */
	QString getRoleADoc() const;

	/**
	 * Returns the documentation assigned to the role B.
	 *
	 * @return	Documentation text of role B.
	 */
	QString getRoleBDoc() const;

	/**
	 * Sets the assocType of the UMLAssociation.
	 *
	 * @param assocType	The Association_Type of the UMLAssociation.
	 */
	void setAssocType(Uml::Association_Type assocType);

	/**
	 * Sets the UMLObject playing role A in the association.
	 *
	 * @param obj		Pointer to the UMLObject of role A.
	 */
	void setObjectA(UMLObject *obj);

	/**
	 * Sets the UMLObject playing role B in the association.
	 * @param obj		Pointer to the UMLObject of role B.
	 */
	void setObjectB(UMLObject *obj);

	/**
	 * Sets the visibility of role A of the UMLAssociation.
	 *
	 * @param value 	Scope of role A visibility.
	 */
	void setVisibilityA(Scope value);

	/**
	 * Sets the visibility of role B of the UMLAssociation.
	 *
	 * @param value 	Scope of role A visibility.
	 */
	void setVisibilityB(Scope value);

	/**
	 * Sets the changeability of role A of the UMLAssociation.
	 *
	 * @param value 	Changeability_Type of role A changeability.
	 */
	void setChangeabilityA(Changeability_Type value);

	/**
	 * Sets the changeability of role B of the UMLAssociation.
	 *
	 * @param value 	Changeability_Type of role B changeability.
	 */

	void setChangeabilityB(Changeability_Type value);

	/**
	 * Sets the multiplicity of role A of the UMLAssociation.
	 *
	 * @param multiA	The multiplicity of role A of the UMLAssociation.
	 */
	void setMultiA(QString multiA);

	/**
	 * Sets the multiplicity of role B of the UMLAssociation.
	 *
	 * @param multiB	The multiplicity of role B of the UMLAssociation.
	 */
	void setMultiB(QString multiB);

	/**
	 * Sets the name of role A of the UMLAssociation.
	 *
	 * @param roleNameA	The name of role A of the UMLAssociation.
	 */
	void setRoleNameA(QString roleNameA);

	/**
	 * Sets the documentation on role A in the association.
	 *
	 * @param doc		The string with the documentation.
	 */
	void setRoleADoc(QString doc);

	/**
	 * Sets the name of role B of the UMLAssociation.
	 *
	 * @param roleNameB	The name of role B of the UMLAssociation.
	 */
	void setRoleNameB(QString roleNameB);

	/**
	 * Sets the documentation on role B in the association.
	 *
	 * @param doc	The string with the documentation.
	 */
	void setRoleBDoc(QString doc);

	/**
         * Convert Changeability_Type value into QString representation.
	 *
	 * @param type		The Changeability_Type enum value to convert.
	 */
	static QString ChangeabilityToString(Uml::Changeability_Type type);

        /**
         * Convert Scope value into QString representation.
	 *
	 * @param scope		The Scope enum value to convert.
         */
	static QString ScopeToString(Uml::Scope scope);

	/**
	 * Get the underlying UMLRole object for role A.
	 *
	 * @return	Pointer to the UMLRole object for role A.
	 */
	UMLRole * getUMLRoleA();

	/**
	 * Get the underlying UMLRole object for role B.
	 *
	 * @return	Pointer to the UMLRole object for role B.
	 */
	UMLRole * getUMLRoleB();

	/**
	 * Saves the <UML:Association> XMI element.
	 */
	bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	/**
	 * Loads the <UML:Association> XMI element.
	 */
	bool loadFromXMI(QDomElement& element);

protected:

	/**
	 * Common initializations at construction time.
	 *
	 * @param type		The Association_Type to represent.
	 * @param roleAObj	Pointer to the role A UMLObject.
	 * @param roleBObj	Pointer to the role B UMLObject.
	 */
	void init(Association_Type type, UMLObject *roleAObj, UMLObject *roleBObj);

	/* If the type Uml::Association_Type is changed then also the following
	   must be changed accordingly:
	   atypeFirst, atypeLast, assocTypeStr[], toAssocType(), toString().
	   The ordering within assocTypeStr must match the enumeration
	   order of Uml::Association_Type.
	 */
	static const Uml::Association_Type atypeFirst;
	static const Uml::Association_Type atypeLast;
	static const unsigned nAssocTypes;
	static const QString assocTypeStr[];

	UMLRole * m_pRoleA;
	UMLRole * m_pRoleB;

	Uml::Association_Type m_AssocType;
	QString m_Name;

};

#endif

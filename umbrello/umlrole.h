/***************************************************************************
 *									   *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
 *									   *
 ***************************************************************************/

#ifndef UMLROLE_H
#define UMLROLE_H

#include "umlnamespace.h"
#include "umlobject.h"

class UMLAssociation;

/**
 *	This class contains the non-graphic representation of an association role.
 *
 *	@author Brian Thomas <brian.thomas@gsfc.nasa.gov>
 *	@version 0.1
 *	@see	UMLObject
 */

class UMLRole : public UMLObject {
	Q_OBJECT
public:
	/**
	 *	Sets up an association.
	 *
	 *	@param	parent	The parent of this UMLRole.
	 */
	UMLRole (UMLAssociation * parent);

	/**
	 * 	Overloaded '==' operator
	 */
	bool operator==(UMLRole & rhs);

	/**
	 *	Standard deconstructor.
	 */
	~UMLRole() { }

	/**
	 *	Returns the UMLObject assigned to the role.
	 *	@return		pointer to the UMLObject in role.
	 */
	UMLObject* getObject();

	/**
	 *	Returns the ID of the UMLObject assigned to the role.
	 *
	 *	@return		Pointer to the UMLObject of role.
	 */
	int getId() const;

	/**
	 *	Returns the Changeablity of the role.
	 *
	 *	@return		Changeability_Type of role.
	 */
	Changeability_Type getChangeability() const;

	/**
	 *	Returns the Visibility of the role.
	 *
	 *	@return		Scope of role.
	 */
	Scope getVisibility() const;

	/**
	 *	Returns the multiplicity assigned to the role.
	 *
	 *	@return		The multiplicity assigned to the role.
	 */
	QString getMultiplicity() const;

	/**
	 *	Returns the name assigned to the role.
	 *
	 *	@return		The name assigned to the role.
	 */
	QString getName() const;

	/**
	 *	Returns the documentation for the role.
	 */
	QString getDoc() const;

	/**
	 *	Sets the ID of the UMLObject playing in the role.
	 *	@param	id of the UMLObject of role.
	 */
	void setId(int roleId);

	/**
	 *	Sets the UMLObject playing the role in the association.
	 *	@param	pointer to the UMLObject of role.
	 */
	void setObject(UMLObject *obj);

	/**
	 *	Sets the visibility of the role.
	 *
	 *	@param	value 	Scope of role visibility.
	 */
	void setVisibility(Scope value);

	/**
	 *	Sets the changeability of the role.
	 *
	 *	@param	value 	Changeability_Type of role changeability.
	 */
	void setChangeability (Changeability_Type value);

	/**
	 *	Sets the multiplicity of the role.
	 *
	 *	@param	multi	The multiplicity of role.
	 */
	void setMultiplicity (QString multi);

	/**
	 *	Sets the name of the role.
	 *
	 *	@param	roleName	The name of role.
	 */
	void setName(QString roleName);

	/**
	 * 	Sets the documentation on the role.
	 * 	@param	doc	The string with the documentation.
	 */
	void setDoc(QString doc);

	UMLAssociation * getParentAssociation ();

	// bool saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

	// bool loadFromXMI(QDomElement& element);

protected:

	/** do some initialization at construction time */
	void init ();

	/**
	 * If the type Uml::Association_Type is changed then also the following
	 * must be changed accordingly:
	 * atypeFirst, atypeLast, assocTypeStr[], toAssocType(), toString().
	 * The ordering within assocTypeStr must match the enumeration
	 * order of Uml::Association_Type.
	 */

	int m_Id;
	QString m_Doc;
	UMLObject* m_pObject;
	QString m_Name;
	QString m_Multi;
	Scope m_Visibility;
	Changeability_Type m_Changeability;

};

#endif

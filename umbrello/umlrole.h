 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

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

#include "umlobject.h"

class UMLAssociation;

/**
 * This class contains the non-graphic representation of an association role.
 *
 * @author Brian Thomas <brian.thomas@gsfc.nasa.gov>
 * @version $Revision$
 * @see	UMLObject
 */

class UMLRole : public UMLObject {
	Q_OBJECT
public:
	/**
	 * Sets up an association.
	 *
	 * @param parent	The parent of this UMLRole.
	 */
	UMLRole (UMLAssociation * parent, UMLObject * parentUMLObject, Uml::Role_Type roleId);

	/**
	 * Overloaded '==' operator
	 */
	bool operator==(UMLRole & rhs);

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLRole();

	/**
	 * Returns the UMLObject assigned to the role.
	 * @return	Pointer to the UMLObject in role.
	 */
	UMLObject* getObject();

	/**
	 * Returns the Changeablity of the role.
	 *
	 * @return	Changeability_Type of role.
	 */
	Uml::Changeability_Type getChangeability() const;

	/**
	 * Returns the Visibility of the role.
	 *
	 * @return	Scope of role.
	 */
	Uml::Scope getVisibility() const;

	/**
	 * Returns the multiplicity assigned to the role.
	 *
	 * @return	The multiplicity assigned to the role.
	 */
	QString getMultiplicity() const;

	/**
	 * Returns the name assigned to the role.
	 *
	 * @return	The name assigned to the role.
	 */
	QString getName() const;

	/**
	 * Returns the documentation for the role.
	 *
	 * @return	The documentation text for the role.
	 */
	QString getDoc() const;

	/**
	 * Sets the UMLObject playing the role in the association.
	 *
	 * @param obj		Pointer to the UMLObject of role.
	 */
	void setObject(UMLObject *obj);

	/**
	 * Sets the visibility of the role.
	 *
	 * @param value 	Scope of role visibility.
	 */
	void setVisibility(Uml::Scope value);

	/**
	 * Sets the changeability of the role.
	 *
	 * @param value 	Changeability_Type of role changeability.
	 */
	void setChangeability (Uml::Changeability_Type value);

	/**
	 * Sets the multiplicity of the role.
	 *
	 * @param multi		The multiplicity of role.
	 */
	void setMultiplicity (QString multi);

	/**
	 * Sets the name of the role.
	 *
	 * @param roleName	The name of role.
	 */
	void setName(QString roleName);

	/**
	 *  Sets the documentation on the role.
	 *
	 *  @param doc		The string with the documentation.
	 */
	void setDoc(QString doc);

	UMLAssociation * getParentAssociation ();

	/** overload UMLObject methods..we want to slave the ID of the
	 * UMLRole to that of the UMLObject which owns it. The wrinkle
	 * here is that we CANT allow UMLRole to be the parent object
	 * of a UMLRole, got it?
	 */
	virtual Uml::IDType getID() const;
	virtual void setID ( Uml::IDType id);

	void setIdStr(QString idStr);

	/** get the 'id' of the role (NOT the parent object). This could be
	 * either Uml::A or Uml::B. Yes, it would be better if we
	 * could get along without this, but we need it to distinguish saved
	 * umlrole objects in the XMI for 'self' associations where both roles
	 * will point to the same underlying UMLObject.
	 */
	Uml::Role_Type getRoleID();

	/**
	 * Make a clone of this object.
	 * Not yet implemented.
	 */
	UMLObject* clone() const { return NULL; }

	/**
	 * Creates the <UML:AssociationEnd> XMI element.
	 */
	void saveToXMI(QDomDocument& qDoc, QDomElement& qElement);

protected:
	/**
	 * Loads the <UML:AssociationEnd> XMI element.
	 * Auxiliary to UMLObject::loadFromXMI.
	 */
	bool load(QDomElement& element);

private:

	/** do some initialization at construction time */
	void init (UMLAssociation * parent, UMLObject * parentObj, Uml::Role_Type r);

	QString m_Doc;
	UMLAssociation * m_pAssoc;
	Uml::Role_Type m_roleID;
	QString m_Name;
	QString m_Multi;
	Uml::Changeability_Type m_Changeability;

};

#endif

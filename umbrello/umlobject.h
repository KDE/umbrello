/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLOBJECT_H
#define UMLOBJECT_H

//qt includes
#include <qobject.h>
#include <qstring.h>
#include <qdom.h>

#include "umlnamespace.h"

using namespace Uml;

/**
 *	This class is the non-graphical version of @ref UMLWidget.  These are
 *	created and maintained in the class @ref UMLDoc.  This class holds all
 *	the generic information needed for all UMLObjects.
 *
 *	@short	The base class for UML objects.
 *	@author Paul Hensgen	<phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLObject : public QObject {
	Q_OBJECT
public:

	/**
	 * Creates a UMLObject.
	 *
	 * @param	parent		The parent of the object.
	 * @param	name		The name of the object.
	 * @param	id		The ID of the object.
	 */
	UMLObject(QObject * parent, QString Name, int id);

	/**
	 * Creates a UMLObject.
	 *
	 * @param	parent		The parent of the object.
	 */
	UMLObject(QObject * parent);

	/**
	 * Overloaded '==' operator
	 */
	virtual bool operator==(UMLObject & rhs );

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLObject();

	/**
	 * Returns the type of the object.
	 *
	 * @return	Returns the type of the object.
	 */
	UMLObject_Type getBaseType() const;

	/**
	 * Returns the ID of the object.
	 *
	 * @return	Returns the ID of the object.
	 */
	int getID() const;

	/**
	 * Sets the documentation for the object.
	 *
	 * @param	d	The documentation for the object.
	 */
	void setDoc(QString d);

	/**
	 * Returns the documentation for the object.
	 *
	 * @return	Returns the documentation for the object.
	 */
	QString getDoc() const;

	/**
	 * Returns the scope of the object.
	 *
	 * @return	Returns the scope of the object.
	 */
	Scope getScope() const;

	/**
	 * Sets the scope of the object.
	 *
	 * @param	s	The scope of the object.
	 */
	void setScope(Scope s);

	/**
	 * Sets the classes Stereotype.
	 *
	 * @param _name	Sets the classes Stereotype name.
	 */
	void setStereotype(QString _name);

	/**
	 * Sets the classes Package.
	 *
	 * @param _name	The classes Package name.
	 */
	void setPackage(QString _name);

	/**
	 * Returns the classes Stereotype.
	 *
	 * @return	Returns the classes Stereotype.
	 */
	QString getStereotype();

	/**
	 * Returns the classes Package.
	 *
	 * @return	Returns the classes Package.
	 */
	QString getPackage();

	/**
	 * Assigns a new Id to the object
	 */
	void setID(int NewID);

	/**
	 * Returns a copy of m_Name
	 */
	QString getName() const;

	/**
	 * Set the UMlObject's name
	 */
	void setName(QString strName);

	/**
	 * Returns the abstract state of the object.
	 */
	bool getAbstract() const;

	/**
	 * Sets the abstract state of the object.
	 */
	void setAbstract(bool bAbstract);

	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	virtual bool loadFromXMI( QDomElement & element );

	/**
	 * Returns true if this UMLObject has classifier scope,
	 * otherwise false (the default).
	 */
	bool getStatic() const;


	/**
	 * Sets the value for m_bStatic.
	 */
	void setStatic(const bool bStatic);

	/**
	 * This should be reimplemented by subclasses if they wish to
	 * accept certain types of associations. Note that this only
	 * tells if this UMLObject can accept the association
	 * type. When creating an association another check is made to
	 * see if the associaiton is valid. For example a UMLClass
	 * (UMLClassifier) can accept generalizations and should
	 * return true. If while creating a generalization the
	 * superclass is already subclassed from this, the association
	 * is not valid and will not be created.  The default accepts
	 * nothing (returns false)
	 */
	virtual bool acceptAssociationType(Uml::Association_Type);

public slots:
	/**
	 * Forces the emition of the modified signal.  Useful when
	 * updating several attributes at a time: you can block the
	 * signals, update all atts, and then force the signal.
	 */
	void emitModified();

signals:
	void modified();

protected:
	/**
	 *   The objects id.
	 */
	int m_nId;

	/**
	 * The objects documentation.
	 */
	QString m_Doc;

	/**
	 * The package the object belongs to if applicable.
	 */
	QString m_Package;

	/**
	 * The stereotype of the object if applicable.
	 */
	QString m_Stereotype;

	/**
	 * The objects name.
	 */
	QString m_Name;

	/**
	 * The objects type.
	 */
	UMLObject_Type m_BaseType;

	/**
	 * The objects scope.
	 */
	Scope m_Scope;

	/**
	 * The state of whether the object is abstract or not.
	 */
	bool m_bAbstract;

	/**
	 * This attribute holds whether the UMLObject has instance scope
	 * (false - the default) or classifier scope (true).
	 */
	bool m_bStatic;
};

#endif

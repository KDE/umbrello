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
 *	@version	1.0
 */
class UMLObject : public QObject {

	Q_OBJECT
	friend class UMLWidget;
public:

	/**
	 *	Creates a UMLObject.
	 *
	 *	@param	parent		The parent of the object.
	 *	@param	name		The name of the object.
	 *	@param	id			The ID of the object.
	 */
	UMLObject(QObject * parent, QString Name, int id);

	/**
	 *	Creates a UMLObject.
	 *
	 *	@param	parent		The parent of the object.
	 */
	UMLObject(QObject * parent);

	/**
	 * 		Overloaded '==' operator
	 */
	virtual bool operator==(UMLObject & rhs );

	/**
	 *	Standard deconstructor.
	 */
	~UMLObject();

	/**
	 *	Returns the type of the object.
	 *
	 *	@return	Returns the type of the object.
	 */
	UMLObject_Type getBaseType() {
		return m_BaseType;
	}

	/**
	 *	Returns the ID of the object.
	 *
	 *	@return	Returns the ID of the object.
	 */
	int getID() {
		return m_nId;
	}

	/**
	 *	Sets the documentation for the object.
	 *
	 *	@param	d	The documentation for the object.
	 */
	void setDoc(QString d) {
		m_Doc = d;
	}

	/**
	 *	Returns the documentation for the object.
	 *
	 *	@return	Returns the documentation for the object.
	 */
	QString getDoc() {
		return m_Doc;
	}

	/**
	 *	Returns the scope of the object.
	 *
	 *	@return	Returns the scope of the object.
	 */
	Scope getScope() {
		return m_Scope;
	}

	/**
	 *	Sets the scope of the object.
	 *
	 *	@param	s	The scope of the object.
	 */
	void setScope(Scope s) {
		m_Scope = s;
	}

	/**
	 * Use to save or load this classes information
	 *
	 *	@param	s	Pointer to the datastream (file) to save/load from.
	 *	@param	archive	If true will save the classes information, else will
	 *                      load the information.
	 *	@param	fileversion the version of the serialize format
	 *
	 *	@return	Returns the status of the operation.
	 */
	virtual bool serialize(QDataStream *s, bool archive, int fileversion);

	/**
	 *	Sets the classes Stereotype.
	 *
	 *	@param _name	Sets the classes Stereotype name.
	 */
	void setStereotype(QString _name) {
		m_Stereotype = _name;
	}

	/**
	 *	Sets the classes Package.
	 *
	 *	@param _name	The classes Package name.
	 */
	void setPackage(QString _name) {
		m_Package = _name;
	}

	/**
	 *	Returns the classes Stereotype.
	 *
	 *	@return	Returns the classes Stereotype.
	 */
	QString getStereotype() {
		return m_Stereotype;
	}

	/**
	 *	Returns the classes Package.
	 *
	 *	@return	Returns the classes Package.
	 */
	QString getPackage() {
		return m_Package;
	}

	/**
	 *   Returns the amount of bytes needed to serialize an instance object to the clipboard
	 */
	virtual long getClipSizeOf();

	/**
	 *    Assigns a new Id to the object
	 */
	void setID(int NewID);

	/**
	 *   Returns a copy of m_Name
	 */
	QString getName();

	/**
	 *    Set the UMlObject's name
	 */
	void setName(QString strName);

	/**
	 *   Returns the abstract state of the object.
	 */
	bool getAbstract();

	/**
	 *     Sets the abstract state of the object.
	 */
	void setAbstract(bool bAbstract);

	virtual bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	virtual bool loadFromXMI( QDomElement & element );

  /** Returns true if this UMLObject has classifier scope, otherwise false (the default). */
  bool getStatic() const;
  /** Sets the value for m_bStatic. */
  void setStatic(const bool bStatic);
  
signals:
	void modified();  

protected:
	/**
	 *   The objects id.
	 */
	int m_nId;

	/**
	 *   The objects documentation.
	 */
	QString m_Doc;

	/**
	 *   The package the object belongs to if applicable.
	 */
	QString m_Package;

	/**
	 *   The stereotype of the object if applicable.
	 */
	QString m_Stereotype;

	/**
	 *     The objects name.
	 */
	QString m_Name;

	/**
	 *     The objects type.
	 */
	UMLObject_Type m_BaseType;

	/**
	 *     The objects scope.
	 */
	Scope m_Scope;

	/**
	 *     The state of whether the object is abstract or not.
	 */
	bool m_bAbstract;

	/**
     * This attribute holds whether the UMLObject has instance scope (false - the default) or classifier scope (true).
     */
     bool m_bStatic;
};

#endif

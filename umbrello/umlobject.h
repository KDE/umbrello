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

class UMLPackage;
class kdbgstream;

/**
 * This class is the non-graphical version of @ref UMLWidget.  These are
 * created and maintained in the class @ref UMLDoc.  This class holds all
 * the generic information needed for all UMLObjects.
 *
 * @short The base class for UML objects.
 * @author Paul Hensgen	<phensgen@techie.com>
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLObject : public QObject {
	Q_OBJECT
public:

	/**
	 * Creates a UMLObject.
	 *
	 * @param parent	The parent of the object.
	 * @param name		The name of the object.
	 * @param id		The ID of the object.
	 */
	UMLObject(UMLObject * parent, const QString &name, int id);

	/**
	 * Creates a UMLObject.
	 *
	 * @param	parent		The parent of the object.
	 */
	UMLObject(UMLObject * parent);

	/**
	 * Creates a UMLObject with a given name and unique ID.
	 *
	 * @param	name		The name of the object.
	 * @param	id		The unique ID of the object.
	 */
	UMLObject(const QString &name = "" , int id = -1);

	/**
	 * Overloaded '==' operator
	 */
	virtual bool operator==(UMLObject & rhs );

	/**
	 * Standard deconstructor.
	 */
	virtual ~UMLObject();

	/**
	 * Copy the internal presentation of this object into the new
	 * object.
	 */
	virtual void copyInto(UMLObject *rhs) const;

	/**
	 * Make a clone of this object.
	 */
	virtual UMLObject* clone() const;

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
	virtual int getID() const;

	/**
	 * Sets the documentation for the object.
	 *
	 * @param d	The documentation for the object.
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
	 * @param s	The scope of the object.
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
	 * DEPRECATED - use SetUMLPackage instead.
	 *
	 * @param _name	The classes Package name.
	 */
	void setPackage(QString _name);

	/**
	 * Sets the UMLPackage in which this class is located.
	 *
	 * @param pPkg		Pointer to the class' UMLPackage.
	 */
	void setUMLPackage(UMLPackage* pPkg);

	/**
	 * Returns the classes Stereotype.
	 *
	 * @return	Returns the classes Stereotype.
	 */
	QString getStereotype();

	/**
	 * Returns the classes package as a text.
	 *
	 * @return	Returns the classes package as a text.
	 */
	QString getPackage();

	/**
	 * Returns the UMLPackage that this class is located in.
	 *
	 * @return	Pointer to the UMLPackage of this class.
	 */
	UMLPackage* getUMLPackage();

	/**
	 * Assigns a new Id to the object
	 */
	virtual void setID(int NewID);

	/**
	 * Returns a copy of m_Name
	 */
	QString getName() const;

	/**
	 * Set the UMLObject's name
	 */
	void setName(QString strName);

	/**
	 * Returns the fully qualified name, i.e. all package prefixes and then m_Name.
	 *
	 * @param separator	The separator string to use.
	 * @return	The fully qualified name of this UMLObject.
	 */
	QString getFullyQualifiedName(QString separator = "::") const;

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

	/**
	 * Gets the value of m_AuxId.
	 * Only used for resolving non-numeric xmi.id's during loading.
	 * All other code should use getID() instead.
	 */
	QString getAuxId() const;

public slots:
	/**
	 * Forces the emission of the modified signal.  Useful when
	 * updating several attributes at a time: you can block the
	 * signals, update all atts, and then force the signal.
	 */
	void emitModified();

signals:
	/** emited when the UMLObject has changed. Note that some objects emit
	  * this signal when one of its children changes, for example, a UMLClass
	  * emits a modified() signal when one of its operation changes while the Operation
	  * itself emits the corresponding signal as well.
	  */
	void modified();
	/** Signals that a new UMLObject has been added to this object.
	  * More specialized classes like UMLPackages or UMLClass emit
	  * more specialized signals, like "classAdded" or "operationAdded" in
	  * addition to this one.
	  */
	void childObjectAdded( UMLObject *obj );
	/** Signals that a UMLObject has been removed from this object.
	  * More specialized classes like UMLPackages or UMLClass emit
	  * more specialized signals, like "classRemoved" or "operationRemoved" in
	  * addition to this one.
	  */
	void childObjectRemoved( UMLObject *obj );

protected:
	/**
	 * Initializes key variables of the class.
	 */
	virtual void init();

	/**
	 * The object's id.
	 */
	int m_nId;

	/**
	 * The object's xmi.id string when non-numeric.
	 * Used for intermediate processing during loading of files
	 * containing non-numeric xmi.id's.
	 * In the end all objects are uniquely identified only by their
	 * numeric id (m_nId).
	 */
	QString m_AuxId;

	/**
	 * The object's documentation.
	 */
	QString m_Doc;

	/**
	 * The package the object belongs to if applicable.
	 */
	UMLPackage* m_pUMLPackage;

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

/**
 * Print UML Object to kdgstream, so it can be used like
 *   kdWarn() << "This object shouldn't be here:" << illegalObject << endl;
 */
kdbgstream& operator<< (kdbgstream& s, const UMLObject& a);

#endif

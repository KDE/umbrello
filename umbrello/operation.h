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
#include "umldoc.h"
#include "classifierlistitem.h"
#include "umlattributelist.h"

/**
 * This class represents an operation in the UML model.
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class UMLOperation : public UMLClassifierListItem {
	Q_OBJECT
friend class UMLClassifier;
friend UMLOperation* UMLDoc::createOperation( UMLClassifier *, const QString &, UMLAttributeList * );
private:
	/**
	 * Constructs an UMLOperation.
	 * Note that the constructor is private - if you want to create an Operation use
	 * the methods in UMLDoc to get one.
	 *
	 * @param parent	The parent to this operation.
	 * @param name		The name of the operation.
	 * @param id		The id of the operation.
	 * @param s		The scope of the operation.
	 * @param rt		The return type of the operation.
	 */
	UMLOperation(UMLClassifier * parent, QString Name, int id, Scope s = Public, QString rt = "void");

	/**
	 * Constructs an UMLOperation.
	 * Note that the constructor is private - if you want to create an Operation use
	 * the methods in UMLDoc to get one.
	 *
	 * @param parent	The parent to this operation.
	 */
	UMLOperation(UMLClassifier * parent);
public:

	/**
	 * destructor
	 */
	virtual ~UMLOperation();

	/**
	 * Overloaded '==' operator.
	 */
	bool operator==( UMLOperation & rhs );

	/**
	 * Copy the internal presentation of this object into the new
	 * object.
	 */
	virtual void copyInto(UMLOperation *rhs) const;

	/**
	 * Make a clone of this object.
	 */
	virtual UMLObject* clone() const;

	/**
	 * Sets the return type of the operation.
	 *
	 * @param rt	The return type to set the operation to.
	 */
	void setReturnType(QString rt) {
		m_ReturnType = rt;
	}

	/**
	 * Get the return type of the operation.
	 *
	 * @return	The return type.
	 */
	QString getReturnType() const {
		return m_ReturnType;
	}

	/**
	 * Add a parameter to the operation.
	 *
	 * @param type		The type of the parameter.
	 * @param name		The name of the parameter.
	 * @param initialValue	The initial value of the parameter.
	 * @param kind		Optional: The parameter kind (in|inout|out).
	 *                      Default is "in".
	 * @return	Pointer to the UMLAttribute created.
	 */
	UMLAttribute * addParm(QString type, QString name, QString initialValue, QString doc,
		               Uml::Parameter_Kind kind = Uml::pk_In);

	/**
	 * Remove a parameter from the operation.
	 *
	 * @param a		The paramter to remove.
	 */
	void removeParm(UMLAttribute *a);

	/**
	 * Returns a list of parameters.
	 *
	 * @return A list of the paramters in the operation.
	 */
	UMLAttributeList* getParmList() {
		return &m_List;
	}

	/**
	 * Finds a parameter of the operation.
	 *
	 * @param name	The parameter name to search for.
	 * @return	The found parameter, 0 if not found.
	 */
	UMLAttribute * findParm(QString name);

	/**
	 * Returns a string representation of the operation.
	 *
	 * @param sig	What type of operation string to show.
	 * @return	The string representation of the operation.
	 */
	QString toString(Signature_Type sig = st_NoSig);

	/**
	 * Add a parameter to the operation.
	 *
	 * @param parameter	The paramter to add.
	 * @param position	The position in the parameter list.
	 *			If position = -1 the parameter will be
	 *			appended to the list.
	 */
	void addParm(UMLAttribute *parameter, int position = -1);

	/**
	 * Returns an unused parameter name for a new parameter.
	 */
	QString getUniqueParameterName();

	/**
	 * Display the properties configuration dialogue for the template.
	 */
	bool showPropertiesDialogue(QWidget* parent);

	/**
	 * Saves to the <UML:Operation> XMI element.
	 */
	void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	/**
	 * Returns whether this operation is a constructor.
	 *
	 * @return	True if this operation is a constructor.
	 */
	bool isConstructorOperation ( ) const;

protected:
	/**
	 * Loads a <UML:Operation> XMI element.
	 */
	bool load( QDomElement & element );

private:
	QString m_ReturnType;
	UMLAttributeList m_List;
};

#endif

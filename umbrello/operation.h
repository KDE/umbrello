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

#include "umldoc.h"
#include "classifierlistitem.h"

class UMLClassifier;

/**
 * This class represents an operation in the UML model.
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLOperation : public UMLClassifierListItem {
	Q_OBJECT
friend UMLOperation* UMLDoc::createOperation( UMLClassifier *, const QString &, UMLAttributeList * );
public:
	/**
	 * Constructs an UMLOperation.
	 * Not intended for general use: The operation is not tied in with
	 * umbrello's Qt signalling for object creation.
	 * If you want to create an Operation use the method in UMLDoc instead.
	 *
	 * @param parent	The parent to this operation.
	 * @param name		The name of the operation.
	 * @param id		The id of the operation.
	 * @param s		The scope of the operation.
	 * @param rt		The return type of the operation.
	 */
	UMLOperation(const UMLClassifier * parent, QString Name, Uml::IDType id = Uml::id_None,
		     Uml::Scope s = Uml::Public, QString rt = "void");

	/**
	 * Constructs an UMLOperation.
	 * Not intended for general use: The operation is not tied in with
	 * umbrello's Qt signalling for object creation.
	 * If you want to create an Operation use the method in UMLDoc instead.
	 *
	 * @param parent	The parent to this operation.
	 */
	UMLOperation(const UMLClassifier * parent);
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
		               Uml::Parameter_Direction kind = Uml::pd_In);

	/**
	 * Move a parameter one position to the left.
	 *
	 * @param a		The parameter to move.
	 */
	void moveParmLeft(UMLAttribute *a);
	
	/**
	 *Move a parameter one position to the right.
	 *
	 * @param a		The parameter to move.
	 */
	void moveParmRight(UMLAttribute *a);
	 
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
	QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

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
	 * Calls resolveRef() on all parameters.
	 * Needs to be called after all UML objects are loaded from file.
	 *
	 * @return	True for success.
	 */
	bool resolveRef();

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
	bool isConstructorOperation();

protected:
	/**
	 * Loads a <UML:Operation> XMI element.
	 */
	bool load( QDomElement & element );

private:
	UMLAttributeList m_List;
};

#endif

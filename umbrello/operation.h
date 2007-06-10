/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef OPERATION_H
#define OPERATION_H

#include "umlattributelist.h"
#include "classifierlistitem.h"

class UMLClassifier;

/**
 * This class represents an operation in the UML model.
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class UMLOperation : public UMLClassifierListItem {
    Q_OBJECT
public:
    /**
     * Constructs an UMLOperation.
     * Not intended for general use: The operation is not tied in with
     * umbrello's Qt signalling for object creation.
     * If you want to create an Operation use the method in UMLDoc instead.
     *
     * @param parent    The parent to this operation.
     * @param name      The name of the operation.
     * @param id        The id of the operation.
     * @param s         The visibility of the operation.
     * @param rt        The return type of the operation.
     */
    UMLOperation(const UMLClassifier * parent, const QString& name,
                 Uml::IDType id = Uml::id_None,
                 Uml::Visibility s = Uml::Visibility::Public,
                 UMLObject *rt = 0);

    /**
     * Constructs an UMLOperation.
     * Not intended for general use: The operation is not tied in with
     * umbrello's Qt signalling for object creation.
     * If you want to create an Operation use the method in UMLDoc instead.
     *
     * @param parent    The parent to this operation.
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
     * Reimplement method from UMLClassifierListItem.
     *
     * @param type      Pointer to the type object.
     */
    void setType(UMLObject *type);

    /**
     * Move a parameter one position to the left.
     *
     * @param a         The parameter to move.
     */
    void moveParmLeft(UMLAttribute *a);

    /**
     *Move a parameter one position to the right.
     *
     * @param a         The parameter to move.
     */
    void moveParmRight(UMLAttribute *a);

    /**
     * Remove a parameter from the operation.
     *
     * @param a         The parameter to remove.
     * @param emitModifiedSignal  Whether to emit the "modified" signal
     *                  which creates an entry in the Undo stack for the
     *                  removal. Default: true.
     */
    void removeParm(UMLAttribute *a, bool emitModifiedSignal = true);

    /**
     * Returns a list of parameters.
     *
     * @return A list of the parameters in the operation.
     */
    UMLAttributeList getParmList() {
        return m_List;
    }

    /**
     * Finds a parameter of the operation.
     *
     * @param name      The parameter name to search for.
     * @return  The found parameter, 0 if not found.
     */
    UMLAttribute * findParm(const QString &name);

    /**
     * Returns a string representation of the operation.
     *
     * @param sig       What type of operation string to show.
     * @return  The string representation of the operation.
     */
    QString toString(Uml::Signature_Type sig = Uml::st_NoSig);

    /**
     * Add a parameter to the operation.
     *
     * @param parameter The parameter to add.
     * @param position  The position in the parameter list.
     *                  If position = -1 the parameter will be
     *                  appended to the list.
     */
    void addParm(UMLAttribute *parameter, int position = -1);

    /**
     * Calls resolveRef() on all parameters.
     * Needs to be called after all UML objects are loaded from file.
     *
     * @return  True for success.
     */
    bool resolveRef();

    /**
     * Returns an unused parameter name for a new parameter.
     */
    QString getUniqueParameterName();

    /**
     * Display the properties configuration dialog for the template.
     */
    bool showPropertiesDialog(QWidget* parent);

    /**
     * Returns whether this operation is a constructor.
     *
     * @return  True if this operation is a constructor.
     */
    bool isConstructorOperation();

    /**
     * Returns whether this operation is a destructor.
     *
     * @return  True if this operation is a destructor.
     */
    bool isDestructorOperation();

    /**
     * Shortcut for (isConstructorOperation() || isDestructorOperation())
     *
     * @return  True if this operation is a constructor or destructor.
     */
    bool isLifeOperation();

    /**
     * Sets whether this operation is a query (C++ "const".)
     */
    void setConst(bool b);

    /**
     * Returns whether this operation is a query (C++ "const".)
     */
    bool getConst() const;

    /**
     * Saves to the <UML:Operation> XMI element.
     */
    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

protected:
    /**
     * Loads a <UML:Operation> XMI element.
     */
    bool load( QDomElement & element );

private:
    Uml::IDType m_returnId;   ///< Holds the xmi.id of the <UML:Parameter kind="return">
    UMLAttributeList m_List;   /// Parameter list
    bool m_bConst;   ///< Holds the isQuery attribute of the <UML:Operation>
};

#endif

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

//
// C++ Interface: valawriter
//

#ifndef VALAWRITER_H
#define VALAWRITER_H

#include "classifierlistitem.h"
#include "simplecodegenerator.h"
#include "umlassociationlist.h"
#include "umlattributelist.h"
#include "umloperationlist.h"


/**
 * Class ValaWriter is a Vala code generator for UMLClassifier objects
 * Just call writeClass and feed it a UMLClassifier;
 */
class ValaWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:
    ValaWriter();

    virtual ~ValaWriter();

    /**
     * Call this method to generate Php code for a UMLClassifier
     * @param c the class you want to generate code for.
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * Returns "Vala".
     */
    virtual Uml::Programming_Language getLanguage();

    /**
     * Get list of reserved keywords.
     */
    virtual const QStringList reservedKeywords() const;

    /**
     * Get list of predefined data types.
     */
    QStringList defaultDatatypes();

private:

    /**
     * Adds extra indenting if the class is in a container (namespace).
     */
    QString m_container_indent;

    /**
     * Collection of included namespaces, to skip them from variable types.
     */
    UMLPackageList m_seenIncludes;

    /**
     * Counts associations without a role name for giving a default name.
     */
    int m_unnamedRoles;

    /**
     * Write realizations of a class and recurse to parent classes.
     * @param currentClass class to start with
     * @param realizations realizations of this class
     * @param cs output stream
     */
    void writeRealizationsRecursive(UMLClassifier *currentClass,
                                    UMLAssociationList *realizations,
                                    QTextStream &cs);

    /**
     * Write all operations for a given class.
     * @param c the concept we are generating code for
     * @param cs output stream
     */
    void writeOperations(UMLClassifier *c, QTextStream &cs);

    /**
     * Write a list of class operations.
     * @param opList the list of operations
     * @param cs output stream
     * @param interface indicates if the operation is an interface member
     * @param isOverride implementation of an inherited abstract function
     */
    void writeOperations(UMLOperationList opList,
                         QTextStream &cs,
                         bool interface = false,
                         bool isOverride = false,
                         bool generateErrorStub = false);

    /**
     * Write superclasses' abstract methods.
     * @param superclasses List of superclasses to start recursing on
     * @param cs output stream
     */
    void writeOverridesRecursive(UMLClassifierList *superclasses, QTextStream &cs);

    /**
     * Write all the attributes of a class.
     * @param c the class we are generating code for
     * @param cs output stream
     */
    void writeAttributes(UMLClassifier *c, QTextStream &cs);

    /**
     * Write a list of class attributes.
     * @param atList the list of attributes
     * @param cs output stream
     */
    void writeAttributes(UMLAttributeList &atList, QTextStream &cs);

    /**
     * Write attributes from associated objects (compositions, aggregations).
     * @param associated list of associated objects
     * @param c currently written class, to see association direction
     * @param cs output stream
     */
    void writeAssociatedAttributes(UMLAssociationList &associated, UMLClassifier *c, QTextStream &cs);

    /**
     * Write a single attribute to the output stream.
     * @param doc attribute documentation
     * @param visibility attribute visibility
     * @param isStatic static attribute
     * @param typeName class/type of the attribute
     * @param name name of the attribute
     * @param initialValue initial value given to the attribute at declaration
     * @param asProperty true writes as property (get/set), false writes single line variable
     * @param cs output stream
     */
    void writeAttribute(const QString& doc,
                        Uml::Visibility visibility,
                        bool isStatic,
                        const QString& typeName,
                        const QString& name,
                        const QString& initialValue,
                        bool asProperty,
                        QTextStream &cs);

    /**
     * Find the type in used namespaces, if namespace found return short name, complete otherwise.
     * @param at Operation or Attribute to check type
     */
    QString makeLocalTypeName(UMLClassifierListItem *cl);

};

#endif

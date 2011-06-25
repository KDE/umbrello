/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

/***************************************************************************
    This is the "old" code generator that does not support code editing
    in the Modeller but uses significantly less file space because the
    source code is not replicated in the XMI file.
 ***************************************************************************/

#ifndef DWRITER_H
#define DWRITER_H

#include "simplecodegenerator.h"
#include "umloperationlist.h"
#include "umlattributelist.h"
#include "umlassociationlist.h"

class UMLOperation;

/**
 * Class DWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate a d source file for
 * that concept.
 */
class DWriter : public SimpleCodeGenerator
{
public:

    /**
     * Constructor, initialises a couple of variables.
     */
    DWriter();

    /**
     * Destructor, empty.
     */
    virtual ~DWriter();

    /**
     * Call this method to generate d code for a UMLClassifier.
     * @param c   the class to generate code for
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * Returns "D".
     * @return   the programming language identifier
     */
    virtual Uml::ProgrammingLanguage language() const;

    /**
     * Return the default datatypes.
     * (Overrides method from class CodeGenerator.)
     * @return   list of default datatypes
     */
    QStringList defaultDatatypes();

private:

    /**
     * Writes the module declaration.
     */
    void writeModuleDecl(UMLClassifier *c, QTextStream &d);

    /**
     * Writes the module imports.
     */
    void writeModuleImports(UMLClassifier *c, QTextStream &d);

    /**
     * Writes class's documentation then the class header
     * "public abstract class Foo extents {".
     */
    void writeClassDecl(UMLClassifier *c, QTextStream &d);

    /**
     * Writes the comment and class constructor.
     */
    void writeConstructor(UMLClassifier *c, QTextStream &d);

    /**
     * Return true if the two operations have the same name and the same parameters.
     * @param op1   first operation to be compared
     * @param op2   second operation to be compared
     */
    static bool compareDMethod(UMLOperation *op1, UMLOperation *op2);

    /**
     * Return true if the operation is in the list.
     * @param umlOp   operation to be searched
     * @param opl     list of operations
     */
    static bool dMethodInList(UMLOperation *umlOp, UMLOperationList &opl);

    /**
     * Get all operations which a given class inherit from all its super interfaces and get all operations
     * which this given class inherit from all its super classes.
     * @param c                       the class for which we are generating code
     * @param yetImplementedOpList    the list of yet implemented operations
     * @param toBeImplementedOpList   the list of to be implemented operations
     * @param noClassInPath           tells if there is a class between the base class and the current interface
     */
    void getSuperImplementedOperations(UMLClassifier *c, UMLOperationList &yetImplementedOpList,
                                       UMLOperationList &toBeImplementedOpList, bool noClassInPath = true);

    /**
     * Get all operations which a given class inherit from all its super interfaces and that should be implemented.
     * @param c     the class for which we are generating code
     * @param opl   the list of operations used to append the operations
     */
    void getInterfacesOperationsToBeImplemented(UMLClassifier *c, UMLOperationList &opl);

    /**
     * Write all operations for a given class.
     * @param c   the class for which we are generating code
     * @param j   the stream associated with the output file
     */
    void writeOperations(UMLClassifier *c, QTextStream &j);

    /**
     * Write a list of operations for a given class.
     * @param list   the list of operations you want to write
     * @param j      the stream associated with the output file
     */
    void writeOperations(UMLOperationList &list, QTextStream &j);

    /**
     * Write all attributes for a given class.
     * @param c   the class for which we are generating code
     * @param j   the stream associated with the output file
     */
    void writeAttributes(UMLClassifier *c, QTextStream &j);

    /**
     * Writes the protection modifier line.
     * @param visibility   protection modifier
     * @param d            text stream
     */
    void writeProtectionMod(Uml::Visibility visibility, QTextStream &d);

    /**
     * Writes attribute declarations with a specific
     * protection modifier.
     * @param prot     the protection modifier
     * @param atlist   attribute list
     * @param d        text stream
     */
    void writeAttributeDecl(Uml::Visibility visibility, UMLAttributeList &atlist, QTextStream &d);

    /**
     * Writes the Attribute declarations.
     * @param atpub    List of public attributes
     * @param atprot   list of protected attributes
     * @param atpriv   list of private attributes
     * @param d        text stream
     */
    void writeAttributeDecls(UMLAttributeList &atpub, UMLAttributeList &atprot,
                             UMLAttributeList &atpriv, QTextStream &d );

    /**
     * Searches a list of associations for appropriate ones to write out as attributes.
     */
    void writeAssociationDecls(UMLAssociationList associations, Uml::IDType id, QTextStream &d);

    /**
     * Writes out an association as an attribute using Vector.
     */
    void writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi,
                                  QString doc, Uml::Visibility visib, QTextStream &d);

    /**
     * Calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in atpub.
     */
    void writeAttributeMethods(UMLAttributeList &atpub, Uml::Visibility visibility, QTextStream &d);

    /**
     * Calls @ref writeAssociationRoleMethod() on each of the associations in the given list.
     */
    void writeAssociationMethods(UMLAssociationList associations, UMLClassifier *thisClass,
                                 QTextStream &d);

    /**
     * Calls @ref writeSingleAttributeAccessorMethods() or @ref
     * writeVectorAttributeAccessorMethods() on the assocaition
     * role.
     */
    void writeAssociationRoleMethod(QString fieldClassName, QString roleName, QString multi,
                                    QString description, Uml::Visibility visib, Uml::Changeability change,
                                    QTextStream &d);

    /**
     * Writes getFoo() and setFoo() accessor methods for the attribute.
     */
    void writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
            QString fieldName, QString description,
            Uml::Visibility visibility, Uml::Changeability change,
            bool isFinal, QTextStream &d);

    /**
     * Writes addFoo() and removeFoo() accessor methods for the Vector attribute.
     */
    void writeVectorAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
            QString fieldName, QString description,
            Uml::Visibility visibility, Uml::Changeability change,
            QTextStream &d);

    /**
     * Writes a // style comment.
     */
    void writeComment(const QString &text, const QString &indent, QTextStream &d, bool dDocStyle=false);

    /**
     * Writes a documentation comment.
     */
    void writeDocumentation(QString header, QString body, QString end, QString indent, QTextStream &d);

    /**
     * Returns the name of the given object (if it exists).
     */
    QString getUMLObjectName(UMLObject *obj);

    /**
     * Lowers the case of the first letter in the given string.
     */
    QString deCapitaliseFirstLetter(const QString& str);

    /**
     * Returns the plural form of a subject.
     */
    QString pluralize(const QString& string);

    /**
     * Returns the non-plural form of a subject.
     */
    QString unPluralize(const QString& str);

    /**
     * Replaces `string' with `String' and `bool' with `boolean'.
     */
    QString fixTypeName(const QString& string);

    /**
     * Check that initial values of strings have quotes around them.
     */
    QString fixInitialStringDeclValue(const QString& val, const QString& type);

    /**
     * Write a blank line.
     */
    void writeBlankLine(QTextStream& d);

    /**
     * A \n, used at the end of each line.
     */
    QString startline;

    /**
     * Whether or not this concept is an interface.
     */
    bool isInterface;

};


#endif // DWRITER_H


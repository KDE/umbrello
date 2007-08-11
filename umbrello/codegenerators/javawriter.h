/***************************************************************************
                          javawriter.h  -  description
    This is the "old" code generator that does not support code editing
    in the Modeller but uses significantly less file space because the
    source code is not replicated in the XMI file.
                             -------------------
    copyright            : (C) 2003 Brian Thomas
         (C) 2004  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JAVAWRITER_H
#define JAVAWRITER_H

#include "simplecodegenerator.h"
#include "../umloperationlist.h"
#include "../umlattributelist.h"
#include "../umlassociationlist.h"

class UMLOperation;

/**
  * class JavaWriter is a code generator for UMLClassifier objects.
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate a java source file for
  * that concept
  */
class JavaWriter : public SimpleCodeGenerator {
public:

    /**
     * Constructor, initialises a couple of variables
     */
    JavaWriter();

    /**
     * Destructor, empty
     */
    virtual ~JavaWriter();

    /**
     * call this method to generate java code for a UMLClassifier
     * @param c the class to generate code for
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * returns "Java"
     */
    virtual Uml::Programming_Language getLanguage();

    /**
     * Overrides method from class CodeGenerator
     */
    QStringList defaultDatatypes();

private:

    /**
     * Writes class's documentation then the class header
     * public abstract class Foo extents {
     */
    void writeClassDecl(UMLClassifier *c, QTextStream &java);

    /**
     * Writes the comment and class constructor
     */
    void writeConstructor(UMLClassifier *c, QTextStream &java);

    /**
     * return true if the two operations have the same name and the same parameters
     * @param op1 first operation to be compared
     * @param op2 second operation to be compared
     */
    static bool compareJavaMethod(UMLOperation *op1, UMLOperation *op2);

    /**
     * return true if the operation is in the list
     * @param umlOp operation to be searched
     * @param opl list of operations
     */
    static bool javaMethodInList(UMLOperation *umlOp, UMLOperationList &opl);

    /**
     * get all operations which a given class inherit from all its super interfaces and get all operations
     * which this given class inherit from all its super classes
     * @param c the class for which we are generating code
     * @param yetImplementedOpList the list of yet implemented operations
     * @param toBeImplementedOpList the list of to be implemented operations
     * @param noClassInPath tells if there is a class between the base class and the current interface
     */
    void getSuperImplementedOperations(UMLClassifier *c, UMLOperationList &yetImplementedOpList ,UMLOperationList &toBeImplementedOpList, bool noClassInPath = true);

    /**
     * get all operations which a given class inherit from all its super interfaces and that should be implemented
     * @param c the class for which we are generating code
     * @param opl the list of operations used to append the operations
     */
    void getInterfacesOperationsToBeImplemented(UMLClassifier *c, UMLOperationList &opl);

     /**
     * write all operations for a given class
     * @param c the class for which we are generating code
     * @param j the stream associated with the output file
     */
    void writeOperations(UMLClassifier *c, QTextStream &j);

    /**
     * write a list of operations for a given class
     * @param list the list of operations you want to write
     * @param j the stream associated with the output file
     */
    void writeOperations(UMLOperationList &list, QTextStream &j);

    /**
     * write all attributes for a given class
     * @param c the class for which we are generating code
     * @param j the stream associated with the output file
     */
    void writeAttributes(UMLClassifier *c, QTextStream &j);

    /**
     * writes the Attribute declarations
     * @param atpub List of public attributes
     * @param atprot list of protected attributes
     * @param atpriv list of private attributes
     * @param java text stream
     */
    void writeAttributeDecls(UMLAttributeList &atpub, UMLAttributeList &atprot,
                             UMLAttributeList &atpriv, QTextStream &java );

    /**
     * Searches a list of associations for appropriate ones to write out as attributes
     */
    void writeAssociationDecls(UMLAssociationList associations, Uml::IDType id, QTextStream &java);

    /**
     * Writes out an association as an attribute using Vector
     */
    void writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi,
                                  QString doc, Uml::Visibility visib, QTextStream &java);

    /**
     * calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in atpub
     */
    void writeAttributeMethods(UMLAttributeList &atpub, Uml::Visibility visibility, QTextStream &java);

    /**
     * calls @ref writeAssociationRoleMethod() on each of the associations in the given list
     */
    void writeAssociationMethods(UMLAssociationList associations, UMLClassifier *thisClass,
                                 QTextStream &java);

    /**
     * calls @ref writeSingleAttributeAccessorMethods() or @ref
     * writeVectorAttributeAccessorMethods() on the assocaition
     * role
     */
    void writeAssociationRoleMethod(QString fieldClassName, QString roleName, QString multi,
                                    QString description, Uml::Visibility visib, Uml::Changeability_Type change,
                                    QTextStream &java);

    /**
     * Writes getFoo() and setFoo() accessor methods for the attribute
     */
    void writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
            QString fieldName, QString description,
            Uml::Visibility visibility, Uml::Changeability_Type change,
            bool isFinal, QTextStream &java);

    /**
     * Writes addFoo() and removeFoo() accessor methods for the Vector attribute
     */
    void writeVectorAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
            QString fieldName, QString description,
            Uml::Visibility visibility, Uml::Changeability_Type change,
            QTextStream &java);

    /**
     * Writes a // style comment
     */
    void writeComment(const QString &text, const QString &indent, QTextStream &java, bool javaDocStyle=false);

    /**
     * Writes a documentation comment
     */
    void writeDocumentation(QString header, QString body, QString end, QString indent, QTextStream &java);

    /**
     * Returns the name of the given object (if it exists)
     */
    QString getUMLObjectName(UMLObject *obj);

    /**
     * Replaces `string' with `String' and `bool' with `boolean'
     */
    QString fixTypeName(const QString& string);

    /**
     * check that initial values of strings have quotes around them
     */
    QString fixInitialStringDeclValue(QString value, QString type);

    /**
     * Write a blank line
     */
    void writeBlankLine(QTextStream& java);

    /**
     * a little method for converting scope to string value
     */
    QString scopeToJavaDecl(Uml::Visibility scope);

    /**
     * A \n, used at the end of each line
     */
    QString startline;

    /**
     * Whether or not this concept is an interface.
     */
    bool isInterface;

};


#endif // JAVAWRITER_H


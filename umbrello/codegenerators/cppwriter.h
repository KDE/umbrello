/***************************************************************************
                          cppwriter.h  -  description
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

#ifndef CPPWRITER_H
#define CPPWRITER_H

#include "simplecodegenerator.h"
#include "cppcodegenerationpolicy.h"
#include "../umloperationlist.h"
#include "../umlattributelist.h"
#include "../umlassociationlist.h"

class QFile;
class ClassifierInfo;

/**
  * class CppWriter is a code generator for UMLClassifier objects.
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate both a header (.h) and
  * source (.cpp) file for that classifier.
  */
class CppWriter : public SimpleCodeGenerator {
public:

    /**
     * Constructor, initialises a couple of variables
     */
    CppWriter();

    /**
     * Destructor, empty
     */
    virtual ~CppWriter();

    /**
     * call this method to generate cpp code for a UMLClassifier
     * @param c the class to generate code for
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * returns "C++"
     */
    virtual Uml::Programming_Language getLanguage();

    /**
     * Add C++ primitives as datatypes
     */
    QStringList defaultDatatypes();

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

private:

    /**
     * Writes class's documentation to the class header
     * public abstract class Foo extents {
     */
    void writeClassDecl(UMLClassifier *c, QTextStream &cpp);

    /**
     * Writes the comment and class constructor declaration or methods
     */
    void writeConstructorDecls(QTextStream &h);
    void writeConstructorMethods(QTextStream &cpp);

    /**
     * write all field declarations, for both attributes and associations for the
     * given permitted scope.
     */
    void writeFieldDecl(UMLClassifier *c, Uml::Visibility permitScope, QTextStream &stream);

    /**
     * write all method declarations, for attributes and associations
     * for the given permitted scope.
     */
    void writeAccessorMethodDecl(UMLClassifier *c, Uml::Visibility permitScope, QTextStream &stream);

    /**
     * write all operations for a given class
     * @param c the class for which we are generating code
     * @param isHeaderMethod true when writing to a header file, false for body file
     * @param permitScope what type of method to write (by Scope)
     * @param j the stream associated with the output file
     */
    void writeOperations(UMLClassifier *c, bool isHeaderMethod, Uml::Visibility permitScope, QTextStream &j);

    /**
     * write a list of operations for a given class
     * @param list the list of operations you want to write
     * @param isHeaderMethod true when writing to a header file, false for body file
     * @param j the stream associated with the output file
     */
    void writeOperations(UMLOperationList &list, bool isHeaderMethod, QTextStream &j);

    /**
     * write all attributes for a given class
     * @param c the class for which we are generating code
     * @param j the stream associated with the output file
     */
    void writeAttributes(UMLClassifier *c, QTextStream &j);

    /**
     * writes the Attribute declarations
     * @param visibility the visibility of the attribs to print out
     * @param writeStatic whether to write static or non-static attributes out
     * @param stream text stream
     */
    void writeAttributeDecls (Uml::Visibility visibility, bool writeStatic, QTextStream &stream );

    /**
     * Write out fields and operations for this class selected on a particular
     * visibility.
     */
    void writeHeaderFieldDecl(UMLClassifier *c, Uml::Visibility permitVisibility, QTextStream &stream);

    void writeHeaderAttributeAccessorMethods (Uml::Visibility visibility, bool writeStatic, QTextStream &stream );

    void writeHeaderAttributeAccessorMethodDecls(UMLClassifier *c, Uml::Visibility permitVisibility, QTextStream &stream);
    void writeHeaderAccessorMethodDecl(UMLClassifier *c, Uml::Visibility permitScope, QTextStream &stream);


    /**
     * Searches a list of associations for appropriate ones to write out as attributes
     */
    void writeAssociationDecls(UMLAssociationList associations, Uml::Visibility permit, Uml::IDType id, QTextStream &stream);

    /**
     * Writes out an association as an attribute using Vector
     */
    void writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi,
                                  QString doc, QTextStream &stream);

    /**
     * calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in attribs list.
     */
    void writeAttributeMethods(UMLAttributeList *attribs, Uml::Visibility visib, bool isHeaderMethod,
                               bool isStatic,
                               bool writeMethodBody, QTextStream &stream);

    /**
     * calls @ref writeAssociationRoleMethod() on each of the associations in the given list
     */
    void writeAssociationMethods(UMLAssociationList associations, Uml::Visibility permitVisib,
                                 bool isHeaderMethod,
                                 bool writeMethodBody, bool writePointerVar, Uml::IDType id, QTextStream &stream);

    /**
     * calls @ref writeSingleAttributeAccessorMethods() or @ref
     * writeVectorAttributeAccessorMethods() on the association
     * role
     */
    void writeAssociationRoleMethod(const QString &fieldClassName, bool isHeaderMethod, bool writeMethodBody,
                                    const QString &roleName, const QString &multi,
                                    const QString &description, Uml::Changeability_Type change,
                                    QTextStream &stream);

    /**
     * Writes getFoo() and setFoo() accessor methods for the attribute
     */
    void writeSingleAttributeAccessorMethods(
            const QString &fieldClassName, const QString &Name,
            const QString &fieldName, const QString &description,
            Uml::Changeability_Type change,
            bool isHeaderMethod,
            bool isStatic, bool writeMethodBody, QTextStream &cpp);

    /**
     * Writes addFoo() and removeFoo() accessor methods for the Vector attribute
     */
    void writeVectorAttributeAccessorMethods(
            const QString &fieldClassName, const QString &fieldVarName,
            const QString &fieldName, const QString &description,
            Uml::Changeability_Type change,
            bool isHeaderMethod,
            bool writeMethodBody,
            QTextStream &cpp);

    /**
     * Writes a // style comment
     */
    void writeComment(const QString &text, const QString &indent, QTextStream &cpp);

    /**
     * Writes a documentation comment
     */
    void writeDocumentation(QString header, QString body, QString end, QTextStream &cpp);


    /**
     * write the header file for this classifier.
     */
    void writeHeaderFile (UMLClassifier *c, QFile &file);

    /**
     * write the source code body file for this classifier.
     */
    void writeSourceFile (UMLClassifier *c, QFile &file);

    /**
     * utility method to break up a block of text, which has embedded newline chars,
     * and print them to a stream as separate lines of text, indented as directed.
     */
    void printTextAsSeparateLinesWithIndent (const QString &text, const QString &indent,
                                             QTextStream &stream);

    /**
     * Intellegently print out header include/forward decl. for associated classes.
     */
    void printAssociationIncludeDecl (UMLAssociationList list, Uml::IDType this_id, QTextStream &stream);

    /**
     * If needed, write out the method to initialize attributes of our class.
     */
    void writeInitAttibuteMethod (QTextStream &stream);

    /**
     * If needed, write out the declaration for the method to initialize attributes of our class.
     */
    void writeInitAttibuteDecl (QTextStream &stream);

    /**
     * Returns the name of the given object (if it exists)
     */
    QString getUMLObjectName(UMLObject *obj);

    /**
     * Replaces `string' with STRING_TYPENAME.
     */
    QString fixTypeName(const QString &string);

    /**
     * check that initial values of strings have quotes around them
     */
    QString fixInitialStringDeclValue(const QString &value, const QString &type);

    /**
     * Determine what the variable name of this attribute should be.
     */
    QString getAttributeVariableName (UMLAttribute *at);

    /**
     * Write a blank line
     */
    void writeBlankLine(QTextStream &stream);

    /**
     * Return the policy object
     */
    CPPCodeGenerationPolicy *policyExt();

    /**
     * Summary information about current classifier.
     */
    ClassifierInfo * m_classifierInfo;

    QString VECTOR_METHOD_APPEND;
    QString VECTOR_METHOD_REMOVE;
    QString VECTOR_METHOD_INIT;
    QString OBJECT_METHOD_INIT;

    /**
     * Create association methods for class attributes/associations/operations as inline decl in header.
     */
    bool INLINE_ASSOCIATION_METHODS;

    QStringList ObjectFieldVariables;
    QStringList VectorFieldVariables;

};



#endif // CPPWRITER_H

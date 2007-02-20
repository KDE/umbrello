/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

//
// C++ Interface: csharpwriter
//
// @author Ferenc Veres
//
#ifndef CSHARPWRITER_H
#define CSHARPWRITER_H

#include "simplecodegenerator.h"
#include "../umlattributelist.h"
#include "../umloperationlist.h"
#include "../classifierlistitem.h"

/**
  * class CSharpWriter is a C# code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier;
  */
class CSharpWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:
    CSharpWriter();

    virtual ~CSharpWriter();
    /**
      * call this method to generate Php code for a UMLClassifier
      * @param c the class you want to generate code for.
      */
    virtual void writeClass(UMLClassifier *c);

    /**
     * returns "C#"
     */
    virtual Uml::Programming_Language getLanguage();

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

    /**
     * get list of predefined data types
     */
    QStringList defaultDatatypes();

private:

    /**
     * we do not want to write the comment "Private methods" twice
     */
    bool bPrivateSectionCommentIsWritten;

    /**
    * Adds extra indenting if the class is in a container (namespace)
    */
    QString m_container_indent;

    /**
    * Collection of included namespaces, to skip them from variable types.
    */
    UMLPackageList m_seenIncludes;
    
    /**
      * write all operations for a given class
      *
      * @param c the concept we are generating code for
      * @param php output stream for the PHP file
      */
    void writeOperations(UMLClassifier *c, QTextStream &php);

    /**
      * write a list of class operations
      *
      * @param classname the name of the class
      * @param opList the list of operations
      * @param php output stream for the PHP file
      * @param interface indicates if the operation is an interface member
      */
    void writeOperations(const QString &classname, UMLOperationList &opList,
                         QTextStream &php,
                         bool interface = false, bool generateErrorStub = false);

    /** write all the attributes of a class
      * @param c the class we are generating code for
      * @param php output stream for the PHP file
      */
    void writeAttributes(UMLClassifier *c, QTextStream &php);

    /** write a list of class attributes
      * @param atList the list of attributes
      * @param php output stream for the PHP file
      */
    void writeAttributes(UMLAttributeList &atList, QTextStream &php);

    /** find the type in used namespaces, if namespace found return short name, complete otherwise.
      *
      * @param at Operation or Attribute to check type
      */
    QString makeLocalTypeName(UMLClassifierListItem *cl);

};

#endif

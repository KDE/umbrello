//
// C++ Interface: csharpwriter
//
// Description: 
//
//
// Author: Umbrello UML Modeller Authors <uml-devel@lists.sourceforge.net>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CSHARPWRITER_H
#define CSHARPWRITER_H

#include "simplecodegenerator.h"
#include "../umlattributelist.h"
#include "../umloperationlist.h"

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
};

#endif

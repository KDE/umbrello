/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002      Vincent Decorges  <vincent.decorges@eivd.ch>  *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PYTHONWRITER_H
#define PYTHONWRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

/**
  * Class PythonWriter is a python code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier.
  *
  * NOTE: There is a unit test available for this class.
  *       Please, use and adapt it when necessary.
  */
class PythonWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    PythonWriter();
    virtual ~PythonWriter();

    /**
     * Call this method to generate C++ code for a UMLClassifier.
     * @param c   the class you want to generate code for
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * Return the programming language identifier.
     * @return   programming language id
     */
    virtual Uml::ProgrammingLanguage language() const;

    /**
     * Get list of reserved keywords.
     * @return   the list of reserved keywords
     */
    virtual QStringList reservedKeywords() const;

    /**
     * Reimplementation of method from class CodeGenerator
     */
    QStringList defaultDatatypes();

private:

    /**
     * Write all attributes for a given class.
     * @param c    the concept we are generating code for
     * @param py   output stream for the header file
     */
    void writeAttributes(UMLAttributeList atList, QTextStream &py);

    /**
     * Write all operations for a given class.
     * @param c   the concept we are generating code for
     * @param h   output stream for the header file
     */
    void writeOperations(UMLClassifier *c, QTextStream &h);

    /**
     * Write a list of class operations.
     * @param classname   the name of the class
     * @param opList      the list of operations
     * @param h           output stream for the header file
     * @param access      visibility identifier
     */
    void writeOperations(const QString& classname, UMLOperationList &opList,
                         QTextStream &h, Uml::Visibility access);

    bool m_bNeedPass;  ///< True as long as no "pass" has been written
};

#endif //PYTHONWRITER

/***************************************************************************
                          pythonwriter.h  -  description
                             -------------------
    begin                : Sat Dec 21 2002
    author               : Vincent Decorges
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PYTHONWRITER_H
#define PYTHONWRITER_H

#include "simplecodegenerator.h"
#include "../umlattributelist.h"
#include "../umloperationlist.h"

enum Access {PRIVATE, PUBLIC, PROTECTED};

/**
  * class PythonWriter is a python code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier;
  */
class PythonWriter : public SimpleCodeGenerator {
    Q_OBJECT
public:

    PythonWriter();
    virtual ~PythonWriter();

    /**
      * call this method to generate C++ code for a UMLClassifier
      * @param c the class you want to generate code for.
      */
    virtual void writeClass(UMLClassifier *c);

    /**
     * returns "Python"
     */
    virtual Uml::Programming_Language getLanguage();

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

private:

    /**
     * write all attributes for a given class
     *
     * @param c   the concept we are generating code for
     * @param py  output stream for the header file
     */
    void writeAttributes(UMLAttributeList atList, QTextStream &py);

    /**
      * write all operations for a given class
      *
      * @param c the concept we are generating code for
      * @param h output stream for the header file
      */
    void writeOperations(UMLClassifier *c, QTextStream &h);

    /**
      * write a list of class operations
      *
      * @param classname the name of the class
      * @param opList the list of operations
      * @param h output stream for the header file
      */
    void writeOperations(const QString& classname, UMLOperationList &opList,
                         QTextStream &h, Access access);

    bool m_bNeedPass;  ///< True as long as no "pass" has been written
};

#endif //PYTHONWRITER

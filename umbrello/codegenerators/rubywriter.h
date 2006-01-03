/***************************************************************************
                          rubywriter.h  -  description
                             -------------------
    begin                : Mon Jul 18 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RUBYWRITER_H
#define RUBYWRITER_H

#include "simplecodegenerator.h"
#include "../umloperationlist.h"
#include "../umlattributelist.h"

#include <qstringlist.h>

class ClassifierInfo;

/**
  * class RubyWriter is a ruby code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier;
  */
class RubyWriter : public SimpleCodeGenerator {
    Q_OBJECT
public:

    RubyWriter();
    virtual ~RubyWriter();

    /**
      * call this method to generate C++ code for a UMLClassifier
      * @param c the class you want to generate code for.
      */
    virtual void writeClass(UMLClassifier *c);

    /**
     * returns "Ruby"
     */
    virtual Uml::Programming_Language getLanguage();

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

private:
    /**
     * Convert a C++ type such as 'int' or 'QWidget' to 
     * ruby types Integer and Qt::Widget
     *
     * @param cppType the C++ type to be converted
     */
    QString cppToRubyType(QString cppType);

    /**
     * Convert C++ names such as 'm_foobar' or pFoobar to 
     * just 'foobar' for ruby
     *
     * @param cppName the C++ name to be converted
     */
    QString cppToRubyName(QString cppName);

    /**
     * calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in attribs list.
     */
    void writeAttributeMethods(UMLAttributeList *attribs,
                               Uml::Visibility visibility, QTextStream &stream);


    /**
     * write all method declarations, for attributes and associations 
     * for the given permitted scope.
     */
    void writeSingleAttributeAccessorMethods(QString fieldName, QString description, QTextStream &h);

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
    void writeOperations(QString classname, UMLOperationList &opList,
                         Uml::Visibility permitScope, QTextStream &h);

    /**
     * Summary information about current classifier.
     */
    ClassifierInfo * classifierInfo;
};

#endif //RUBYWRITER

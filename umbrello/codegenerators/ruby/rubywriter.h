/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYWRITER_H
#define RUBYWRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

#include <QtCore/QStringList>


/**
 * Class RubyWriter is a ruby code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class RubyWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    RubyWriter();
    virtual ~RubyWriter();

    /**
     * Call this method to generate C++ code for a UMLClassifier.
     * @param c   the class you want to generate code for.
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * Returns "Ruby".
     * @return   the programming language identifier
     */
    virtual Uml::ProgrammingLanguage language() const;

    /**
     * Get list of reserved keywords.
     * @return   the list of reserved keywords
     */
    virtual QStringList reservedKeywords() const;

private:
    /**
     * Convert a C++ type such as 'int' or 'QWidget' to
     * ruby types Integer and Qt::Widget.
     * @param cppType the C++ type to be converted
     */
    QString cppToRubyType(const QString &cppType);

    /**
     * Convert C++ names such as 'm_foobar' or pFoobar to
     * just 'foobar' for ruby.
     * @param cppName the C++ name to be converted
     */
    QString cppToRubyName(const QString &cppName);

    /**
     * Calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in attribs list.
     * @param attribs      the attribute
     * @param visibility   the visibility of the attribute
     * @param stream       output stream to the generated file
     */
    void writeAttributeMethods(UMLAttributeList attribs,
                               Uml::Visibility visibility, QTextStream &stream);

    /**
     * Write all method declarations, for attributes and associations
     * for the given permitted scope.
     * @param fieldName     the field name
     * @param description   the description
     * @param h             output stream to the generated file
     */
    void writeSingleAttributeAccessorMethods(const QString &fieldName,
                                             const QString &description,
                                             QTextStream &h);

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
     */
    void writeOperations(const QString &classname, const UMLOperationList &opList,
                         Uml::Visibility permitScope, QTextStream &h);

};

#endif //RUBYWRITER

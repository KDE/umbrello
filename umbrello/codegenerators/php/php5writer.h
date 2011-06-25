/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002      Heiko Nardmann  <h.nardmann@secunet.de>       *
 *                           Thorsten Kunz   <tk AT bytecrash DOT net>     *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PHP5WRITER_H
#define PHP5WRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

/**
 * Class Php5Writer is a PHP code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class Php5Writer : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    Php5Writer();
    virtual ~Php5Writer();

    /**
     * Call this method to generate Php code for a UMLClassifier.
     * @param c   the class you want to generate code for.
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * Returns "PHP".
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
     * We do not want to write the comment "Private methods" twice.
     */
    bool bPrivateSectionCommentIsWritten;

    /**
     * Write all operations for a given class.
     * @param c     the concept we are generating code for
     * @param php   output stream for the PHP file
     */
    void writeOperations(UMLClassifier *c, QTextStream &php);

    /**
     * Write a list of class operations.
     * @param classname   the name of the class
     * @param opList      the list of operations
     * @param php         output stream for the PHP file
     * @param interface   indicates if the operation is an interface member
     */
    void writeOperations(const QString &classname, UMLOperationList &opList,
                         QTextStream &php,
                         bool interface = false, bool generateErrorStub = false);

    /**
     * Write all the attributes of a class.
     * @param c     the class we are generating code for
     * @param php   output stream for the PHP file
     */
    void writeAttributes(UMLClassifier *c, QTextStream &php);

    /**
     * Write a list of class attributes.
     * @param atList   the list of attributes
     * @param php      output stream for the PHP file
     */
    void writeAttributes(UMLAttributeList &atList, QTextStream &php);
};

#endif //PHP5WRITER


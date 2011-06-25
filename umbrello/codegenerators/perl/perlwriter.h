/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      David Hugh-Jones  <hughjonesd@yahoo.co.uk>    *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef PERLWRITER_H
#define PERLWRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"
#include "umloperationlist.h"

class UMLOperation;
class UMLAttribute;
class UMLClassifier;

/**
 * Class PerlWriter is a Perl code generator for UMLClassifier objects.
 * Just call writeClass and feed it a UMLClassifier.
 */
class PerlWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    PerlWriter();
    virtual ~PerlWriter();

    /**
     * Call this method to generate Perl code for a UMLClassifier.
     * @param c   the class you want to generate code for
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * Returns "Perl".
     * @return   the programming language identifier
     */
    virtual Uml::ProgrammingLanguage language() const;

    /**
     * Get list of reserved keywords.
     * @return   the list of reserved keywords
     */
    virtual QStringList reservedKeywords() const;

    /**
     * Get list of default datatypes.
     * @return   the list of default datatypes
     */
    QStringList defaultDatatypes();

private:

    /**
     * We do not want to write the comment "Private methods" twice
     * not sure whether this is php specific.
     */
    bool bPrivateSectionCommentIsWritten;

    /**
     * Write all operations for a given class.
     * @param c      the concept we are generating code for
     * @param perl   output stream for the Perl file
     */
    void writeOperations(UMLClassifier *c, QTextStream &perl);

    /**
     * Write a list of class operations.
     * @param classname   the name of the class
     * @param opList      the list of operations
     * @param perl        output stream for the Perl file
     */
    void writeOperations(const QString &classname, UMLOperationList &opList,
                         QTextStream &perl);

    /**
     * Write all the attributes of a class.
     * @param c      the class we are generating code for
     * @param perl   output stream for the Perl file
     */
    void writeAttributes(UMLClassifier *c, QTextStream &perl);

    /**
     * Write a list of class attributes.
     * @param atList   the list of attributes
     * @param perl     output stream for the Perl file
     */
    void writeAttributes(UMLAttributeList &atList, QTextStream &perl);

    bool GetUseStatements(UMLClassifier *c, QString &Ret,
                          QString &ThisPkgName);

};

#endif //PERLWRITER

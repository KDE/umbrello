/***************************************************************************
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  copyright (C) 2006-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef PASCALWRITER_H
#define PASCALWRITER_H

#include "simplecodegenerator.h"

class UMLAssociation;
class UMLOperation;
class QTextStream;

/**
 * Pascal class writer.
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org.
 */
class PascalWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    /**
     * Basic Constructor
     */
    PascalWriter ();

    /**
     * Empty Destructor
     */
    virtual ~PascalWriter ();

    /**
     * Call this method to generate Pascal code for a UMLClassifier.
     * @param c   the class to generate code for
     */
    virtual void writeClass (UMLClassifier *c);

    /**
     * Returns "Pascal".
     * @return   the programming language identifier
     */
    virtual Uml::ProgrammingLanguage language() const;

    /**
     * Returns the default datatypes in a list.
     * @return  the list of default datatypes
     */
    QStringList defaultDatatypes();

    /**
     * Check whether the given string is a reserved word for the
     * language of this code generator.
     * @param rPossiblyReservedKeyword   the string to check
     */
    virtual bool isReservedKeyword(const QString & rPossiblyReservedKeyword);

    /**
     * Get list of reserved keywords.
     * @return   the list of reserved keywords
     */
    virtual QStringList reservedKeywords() const;

private:

    /**
     * Write one operation.
     * @param op the class for which we are generating code
     * @param ada the stream associated with the output file
     */
    void writeOperation (UMLOperation *op, QTextStream &ada, bool is_comment = false);

    void computeAssocTypeAndRole (UMLAssociation *a, QString& typeName, QString& roleName);

    bool isOOClass (UMLClassifier *c);

    QString qualifiedName (UMLPackage *p, bool withType = false, bool byValue = false);

    static const QString defaultPackageSuffix;

};

#endif // PASCALWRITER_H


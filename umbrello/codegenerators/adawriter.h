/***************************************************************************
                          adawriter.h  -  description
                             -------------------
    Based on javawriter.h by Luis De la Parra Blum
    begin                : Sat Dec 14 2002
    copyright            : (C) 2002 by Oliver Kellogg
    email                : okellogg@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADAWRITER_H
#define ADAWRITER_H

#include "simplecodegenerator.h"

class UMLAssociation;
class UMLOperation;

/**
  * class AdaWriter is a code generator for UMLClassifier objects.
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate an Ada package spec for
  * that concept
  */
class AdaWriter : public SimpleCodeGenerator {
    Q_OBJECT
public:

    /**
     * Basic Constructor
     */
    AdaWriter ();

    /**
     * Empty Destructor
     */
    virtual ~AdaWriter ();

    /**
     * call this method to generate Ada code for a UMLClassifier
     * @param c the class to generate code for
     */
    virtual void writeClass (UMLClassifier *c);

    /**
     * returns "Ada"
     */
    virtual Uml::Programming_Language getLanguage();

    QStringList defaultDatatypes();

    /**
     * Check whether the given string is a reserved word for the
     * language of this code generator
     *
     * @param rPossiblyReservedKeyword  The string to check.
     * @return true if the keyword is reserved
     */
    virtual bool isReservedKeyword(const QString & rPossiblyReservedKeyword);

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

private:

    /**
     * write one operation
     * @param op the class for which we are generating code
     * @param ada the stream associated with the output file
     */
    void writeOperation (UMLOperation *op, QTextStream &ada, bool is_comment = false);

    /**
     * Compute the type and role name from the given association.
     *
     * @param c         The UMLClassifier for which code is being generated.
     * @param a         The UMLAssociation to analyze.
     * @param typeName  Return value: type name.
     * @param roleName  Return value: role name.
     */
    void computeAssocTypeAndRole (UMLClassifier *c,
                                  UMLAssociation *a,
                                  QString& typeName, QString& roleName);

    bool isOOClass (UMLClassifier *c);

    QString className(UMLClassifier *c, bool inOwnScope = true);

    QString packageName(UMLPackage *p);

    static const QString defaultPackageSuffix;

};

#endif // ADAWRITER_H

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002    Oliver Kellogg <okellogg@users.sourceforge.net> *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ADAWRITER_H
#define ADAWRITER_H

#include "simplecodegenerator.h"

class UMLAssociation;
class UMLOperation;
class QTextStream;

/**
 * Class AdaWriter is a code generator for UMLClassifier objects.
 * Based on javawriter.h by Luis De la Parra Blum.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate an Ada package spec for
 * that concept.
 */
class AdaWriter : public SimpleCodeGenerator 
{
    Q_OBJECT
public:

    AdaWriter();
    virtual ~AdaWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage language() const;

    QStringList defaultDatatypes();

    virtual bool isReservedKeyword(const QString & rPossiblyReservedKeyword);

    virtual QStringList reservedKeywords() const;

private:

    void writeOperation(UMLOperation *op, QTextStream &ada, bool is_comment = false);

    void computeAssocTypeAndRole(UMLClassifier *c,
                                 UMLAssociation *a,
                                 QString& typeName, QString& roleName);

    bool isOOClass(UMLClassifier *c);

    QString className(UMLClassifier *c, bool inOwnScope = true);

    QString packageName(UMLPackage *p);

    static const QString defaultPackageSuffix;

};

#endif // ADAWRITER_H

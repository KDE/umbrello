/***************************************************************************
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  copyright (C) 2006-2013                                                *
 *  Umbrello UML Modeller Authors <umbrello-devel@kde.org>                 *
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
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org.
 */
class PascalWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    PascalWriter();
    virtual ~PascalWriter ();

    virtual void writeClass (UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    QStringList defaultDatatypes();

    virtual bool isReservedKeyword(const QString & rPossiblyReservedKeyword);

    virtual QStringList reservedKeywords() const;

private:

    void writeOperation (UMLOperation *op, QTextStream &ada, bool is_comment = false);

    void computeAssocTypeAndRole (UMLAssociation *a, QString& typeName, QString& roleName);

    bool isOOClass (UMLClassifier *c);

    QString qualifiedName (UMLPackage *p, bool withType = false, bool byValue = false);

    static const QString defaultPackageSuffix;

};

#endif // PASCALWRITER_H


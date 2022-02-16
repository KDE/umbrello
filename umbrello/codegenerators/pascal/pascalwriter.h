/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef PASCALWRITER_H
#define PASCALWRITER_H

#include "simplecodegenerator.h"

class UMLAssociation;
class UMLOperation;
class QTextStream;

/**
 * Pascal class writer.
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org.
 */
class PascalWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    PascalWriter();
    virtual ~PascalWriter ();

    virtual void writeClass (UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    QStringList defaultDatatypes() const;

    virtual bool isReservedKeyword(const QString & rPossiblyReservedKeyword);

    virtual QStringList reservedKeywords() const;

private:

    void writeOperation (UMLOperation *op, QTextStream &ada, bool is_comment = false);

    void computeAssocTypeAndRole (UMLAssociation *a, QString& typeName, QString& roleName);

    bool isOOClass (const UMLClassifier *c);

    QString qualifiedName (UMLPackage *p, bool withType = false, bool byValue = false);

    static const QString defaultPackageSuffix;

};

#endif // PASCALWRITER_H


/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 David Hugh-Jones <hughjonesd@yahoo.co.uk>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

    QStringList defaultDatatypes() const;

private:
    void writeOperations(UMLClassifier *c, QTextStream &perl);
    void writeOperations(const QString &classname, UMLOperationList &opList,
                         QTextStream &perl);

    void writeAttributes(UMLClassifier *c, QTextStream &perl);
    void writeAttributes(UMLAttributeList &atList, QTextStream &perl);

    bool GetUseStatements(UMLClassifier *c, QString &Ret,
                          QString &ThisPkgName);

};

#endif //PERLWRITER

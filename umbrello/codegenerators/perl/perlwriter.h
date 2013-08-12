/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      David Hugh-Jones  <hughjonesd@yahoo.co.uk>    *
 *   copyright (C) 2004-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
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

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

    QStringList defaultDatatypes();

private:

    /**
     * We do not want to write the comment "Private methods" twice
     * not sure whether this is php specific.
     */
    bool bPrivateSectionCommentIsWritten;

    void writeOperations(UMLClassifier *c, QTextStream &perl);
    void writeOperations(const QString &classname, UMLOperationList &opList,
                         QTextStream &perl);

    void writeAttributes(UMLClassifier *c, QTextStream &perl);
    void writeAttributes(UMLAttributeList &atList, QTextStream &perl);

    bool GetUseStatements(UMLClassifier *c, QString &Ret,
                          QString &ThisPkgName);

};

#endif //PERLWRITER

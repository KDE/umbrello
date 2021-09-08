/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Nikolaus Gradwohl <guru@local-guru.net>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef SQLWRITER_H
#define SQLWRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"

//forward declarations
class UMLEntity;
class UMLEntityAttributeList;
class UMLClassifierListItemList;

/**
 * Class SQLWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate a sql source file for
 * that concept.
 */
class SQLWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    SQLWriter();
    virtual ~SQLWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList defaultDatatypes() const;

    virtual QStringList reservedKeywords() const;

protected:
    QStringList m_enumsGenerated;
    UMLEntity* m_pEntity;

    virtual void printEntityAttributes(QTextStream& sql, UMLEntityAttributeList entityAttributeList);
    virtual void printUniqueConstraints(QTextStream& sql, UMLClassifierListItemList constrList);
    virtual void printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList);
    virtual void printCheckConstraints(QTextStream& sql, UMLClassifierListItemList constrList);
    virtual void printIndex(QTextStream& sql, UMLEntity* ent, UMLEntityAttributeList entAttList);
    virtual void printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList);
};

#endif // SQLWRITER_H

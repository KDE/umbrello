/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2002 Oliver Kellogg <okellogg@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef ADAWRITER_H
#define ADAWRITER_H

#include "simplecodegenerator.h"
#include "umlclassifierlist.h"

#include <QMap>

class UMLAssociation;
class UMLOperation;
class QTextStream;
class QFile;

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

    virtual Uml::ProgrammingLanguage::Enum language() const;

    QStringList defaultDatatypes() const;

    virtual bool isReservedKeyword(const QString & rPossiblyReservedKeyword);

    virtual QStringList reservedKeywords() const;

private:

    void declareClass(UMLClassifier *c, QTextStream &ada);

    void writeOperation(UMLOperation *op, QTextStream &ada, bool is_comment = false);

    void computeAssocTypeAndRole(UMLClassifier *c,
                                 UMLAssociation *a,
                                 QString& typeName, QString& roleName);

    static bool isOOClass(const UMLClassifier *c);

    QString className(UMLClassifier *c, bool inOwnScope = true);

    static QString packageName(UMLPackage *p);

    void finalizeRun();

    static const QString defaultPackageSuffix;

    typedef QMap<QString, QFile*> PackageFileMap;

    /**
     * Map package name to QFile.
     * Required for closing opened files in finalizeRun().
     */
    PackageFileMap m_pkgsGenerated;

    /**
     * List of classifiers generated.
     * Required for ensuring order of code generation which
     * satisfies order of dependencies among classifiers.
     */
    UMLClassifierList m_classesGenerated;
};

#endif // ADAWRITER_H

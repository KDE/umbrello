/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef SIMPLECODEGENERATOR_H
#define SIMPLECODEGENERATOR_H

#include "basictypes.h"
#include "codegenerator.h"

#include <QMap>
#include <QString>
#include <QStringList>


/**
 * A simple code generator interface designed to work with
 * the existing codewriters.
 *
 * Code can be entered into a QTextEdit widget in the ClassPropertiesDialog. This
 * code is then stored in the respective UMLOperation, written to the
 * xmi file, and also used when generating the source files.
 * The code fragments are stored into the xmi file in the section "codegeneration"
 * with the tag "sourcecode".
 */
class SimpleCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:

    explicit SimpleCodeGenerator(bool createDirHierarchyForPackages = true);
    virtual ~SimpleCodeGenerator();

    void writeCodeToFile(UMLClassifierList & concepts);
    void writeCodeToFile();

    /**
     * Call this method to generate code for a UMLClassifier
     * @param c the class you want to generate code for.
     */
    virtual void writeClass(UMLClassifier *c) = 0;

protected:
    QString className_;
    QString fileName_;

    QString findFileName(UMLPackage* concept, const QString &ext);
    QString overwritableName(UMLPackage* concept, const QString &name, const QString &ext);
    bool hasDefaultValueAttr(UMLClassifier *c);
    bool hasAbstractOps(UMLClassifier *c);

    QString indent();

    /**
     * Maps UMLObjects to filenames. Used for finding out which file
     * each class was written to.
     */
    QMap<UMLPackage*, QString> m_fileMap;

    /**
     * For some code generators, it does not make much sense to create a
     * directory for each package because that would lead to a rather
     * sparsely populated directory tree (maximum of just one source file
     * per directory.)
     */
    bool m_createDirHierarchyForPackages;

    // old attributes writers will look for
    QString m_indentation;
    int m_indentLevel;
    QString m_endl;

    virtual void initFromParentDocument();

public slots:
    virtual void syncCodeToDocument();

};

#endif // SIMPLECODEGENERATOR_H

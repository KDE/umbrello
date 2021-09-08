/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACODEGENERATOR_H
#define JAVACODEGENERATOR_H

#include "codeviewerstate.h"
#include "advancedcodegenerator.h"
#include "codeblockwithcomments.h"
#include "javaclassifiercodedocument.h"
#include "javaantcodedocument.h"
#include "javacodegenerationpolicy.h"
#include "umldoc.h"

#include <QString>

class CodeViewerDialog;

class JavaCodeGenerator : public AdvancedCodeGenerator
{
    Q_OBJECT
public:

    JavaCodeGenerator();
    virtual ~JavaCodeGenerator();

    void setCreateANTBuildFile(bool buildIt);
    bool getCreateANTBuildFile();

    bool getAutoGenerateAttribAccessors();

    bool getAutoGenerateAssocAccessors();

    static QString getListFieldClassName();

    virtual CodeViewerDialog * getCodeViewerDialog(QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState & state);

    JavaCodeGenerationPolicy * getJavaPolicy();

    CodeDocument * newClassifierCodeDocument(UMLClassifier * classifier);

    Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList defaultDatatypes() const;

    static QString fixTypeName(const QString &string);

    virtual QStringList reservedKeywords() const;

protected:

//    /**
//     * Create the codeblock that will represent the class declaration
//     * for this classifier.
//     */
//    CodeBlockWithComments * createClassDecl (UMLClassifier *c, JavaClassifierCodeDocument * doc);

    JavaANTCodeDocument * newANTCodeDocument();

private:

    bool m_createANTBuildFile;

};

#endif // JAVACODEGENERATOR_H

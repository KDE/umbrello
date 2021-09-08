/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCODEGENERATOR_H
#define DCODEGENERATOR_H

#include "codeviewerstate.h"
#include "advancedcodegenerator.h"
#include "codeblockwithcomments.h"
#include "dclassifiercodedocument.h"
#include "dcodegenerationpolicy.h"
#include "umldoc.h"

#include <QString>

class CodeViewerDialog;

class DCodeGenerator : public AdvancedCodeGenerator
{
    Q_OBJECT
public:

    DCodeGenerator();
    virtual ~DCodeGenerator();

    bool getAutoGenerateAttribAccessors();
    bool getAutoGenerateAssocAccessors();

    static QString getListFieldClassName();

    virtual CodeViewerDialog * getCodeViewerDialog(QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState & state);

    static QString capitalizeFirstLetter(const QString &item);

    DCodeGenerationPolicy * getDPolicy();

    CodeDocument * newClassifierCodeDocument (UMLClassifier * classifier);

    Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList defaultDatatypes() const;

    static QString fixTypeName(const QString &item);

    virtual QStringList reservedKeywords() const;

protected:

//    /**
//     * Create the codeblock that will represent the class declaration
//     * for this classifier.
//     */
//    CodeBlockWithComments * createClassDecl (UMLClassifier *c, DClassifierCodeDocument * doc);

private:

};

#endif // DCODEGENERATOR_H

/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYCODEGENERATOR_H
#define RUBYCODEGENERATOR_H

#include "codeviewerstate.h"
#include "advancedcodegenerator.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodegenerationpolicy.h"
#include "umldoc.h"

#include <QString>

class CodeBlockWithComments;
class CodeViewerDialog;

class RubyCodeGenerator : public AdvancedCodeGenerator
{
    Q_OBJECT
public:

    RubyCodeGenerator();
    virtual ~RubyCodeGenerator();

    bool getAutoGenerateAttribAccessors();
    bool getAutoGenerateAssocAccessors();

    static QString getListFieldClassName();

    virtual CodeViewerDialog * getCodeViewerDialog(QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState & state);

    RubyCodeGenerationPolicy * getRubyPolicy();

    CodeDocument * newClassifierCodeDocument(UMLClassifier * classifier);

    Uml::ProgrammingLanguage::Enum language() const;

    static QString cppToRubyType(const QString &cppType);
    static QString cppToRubyName(const QString &cppName);

    virtual QStringList reservedKeywords() const;

protected:

//    CodeBlockWithComments * createClassDecl(UMLClassifier *c, RubyClassifierCodeDocument * doc);

};

#endif // RUBYCODEGENERATOR_H

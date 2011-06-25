/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYCODEGENERATOR_H
#define RUBYCODEGENERATOR_H

#include "codeviewerstate.h"
#include "codegenerator.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodegenerationpolicy.h"
#include "umldoc.h"

#include <QtCore/QString>

class CodeBlockWithComments;
class CodeViewerDialog;

class RubyCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:

    RubyCodeGenerator();
    RubyCodeGenerator(QDomElement & element);

    virtual ~RubyCodeGenerator();

    bool getAutoGenerateAttribAccessors();
    bool getAutoGenerateAssocAccessors();

    static QString getListFieldClassName();

    virtual CodeViewerDialog * getCodeViewerDialog(QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState state);

    RubyCodeGenerationPolicy * getRubyPolicy();

    CodeDocument * newClassifierCodeDocument(UMLClassifier * classifier);

    Uml::ProgrammingLanguage language() const;

    static QString cppToRubyType(const QString &cppType);
    static QString cppToRubyName(const QString &cppName);

    virtual QStringList reservedKeywords() const;

protected:

//    CodeBlockWithComments * createClassDecl(UMLClassifier *c, RubyClassifierCodeDocument * doc);

};

#endif // RUBYCODEGENERATOR_H

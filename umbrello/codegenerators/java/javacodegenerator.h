/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef JAVACODEGENERATOR_H
#define JAVACODEGENERATOR_H

#include "codeviewerstate.h"
#include "codegenerator.h"
#include "codeblockwithcomments.h"
#include "javaclassifiercodedocument.h"
#include "javaantcodedocument.h"
#include "javacodegenerationpolicy.h"
#include "umldoc.h"

#include <QtCore/QString>

class CodeViewerDialog;

class JavaCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:

    JavaCodeGenerator();
    JavaCodeGenerator(QDomElement & element);

    virtual ~JavaCodeGenerator();

    void setCreateANTBuildFile(bool buildIt);
    bool getCreateANTBuildFile();

    bool getAutoGenerateAttribAccessors();

    bool getAutoGenerateAssocAccessors();

    static QString getListFieldClassName();

    virtual CodeViewerDialog * getCodeViewerDialog(QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState state);

    JavaCodeGenerationPolicy * getJavaPolicy();

    CodeDocument * newClassifierCodeDocument(UMLClassifier * classifier);

    Uml::ProgrammingLanguage language() const;

    virtual QStringList defaultDatatypes();

    static QString fixTypeName(const QString &string);

    virtual QStringList reservedKeywords() const;

protected:

//    /**
//     * Create the codeblock that will represent the class declaration
//     * for this classifier.
//     */
//    CodeBlockWithComments * createClassDecl ( UMLClassifier *c, JavaClassifierCodeDocument * doc);

    JavaANTCodeDocument * newANTCodeDocument();

private:

    void init();

    bool m_createANTBuildFile;
};

#endif // JAVACODEGENERATOR_H

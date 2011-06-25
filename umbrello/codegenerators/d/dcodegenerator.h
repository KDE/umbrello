/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DCODEGENERATOR_H
#define DCODEGENERATOR_H

#include "codeviewerstate.h"
#include "codegenerator.h"
#include "codeblockwithcomments.h"
#include "dclassifiercodedocument.h"
#include "dcodegenerationpolicy.h"
#include "umldoc.h"

#include <QtCore/QString>

class CodeViewerDialog;

class DCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:

    DCodeGenerator();
    DCodeGenerator(QDomElement & element);

    virtual ~DCodeGenerator();

    bool getAutoGenerateAttribAccessors( );
    bool getAutoGenerateAssocAccessors( );

    static QString getListFieldClassName();

    virtual CodeViewerDialog * getCodeViewerDialog( QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState state);

    static QString capitalizeFirstLetter(const QString &item);

    DCodeGenerationPolicy * getDPolicy();

    CodeDocument * newClassifierCodeDocument (UMLClassifier * classifier);

    Uml::ProgrammingLanguage language() const;

    virtual QStringList defaultDatatypes();

    static QString fixTypeName(const QString &item);

    virtual QStringList reservedKeywords() const;

protected:

//    /**
//     * Create the codeblock that will represent the class declaration
//     * for this classifier.
//     */
//    CodeBlockWithComments * createClassDecl ( UMLClassifier *c, DClassifierCodeDocument * doc);

private:

    void init();
};

#endif // DCODEGENERATOR_H

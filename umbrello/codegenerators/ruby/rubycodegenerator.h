/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005      Richard Dale  <>                              *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYCODEGENERATOR_H
#define RUBYCODEGENERATOR_H

#include "codeviewerstate.h"
#include "codegenerator.h"
#include "codeblockwithcomments.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodegenerationpolicy.h"
#include "umldoc.h"

#include <QtCore/QString>

class CodeViewerDialog;

class RubyCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    RubyCodeGenerator ();
    RubyCodeGenerator (QDomElement & element);

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeGenerator ( );

    /**
     * A utility method to get the rubyCodeGenerationPolicy()->getAutoGenerateAttribAccessors() value.
     */
    bool getAutoGenerateAttribAccessors( );

    /**
     * A utility method to get the rubyCodeGenerationPolicy()->getAutoGenerateAssocAccessors() value.
     */
    bool getAutoGenerateAssocAccessors( );

    /**
     * Get the list variable class name to use. For Ruby, we have set this to "Array".
     */
    static QString getListFieldClassName();

    /**
     * Get the editing dialog for this code document
     */
    virtual CodeViewerDialog * getCodeViewerDialog( QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState state);

    /**
     * Utility function for getting the ruby code generation policy.
     */
    RubyCodeGenerationPolicy * getRubyPolicy();

    /**
     * @return    ClassifierCodeDocument
     * @param    classifier
     */
    CodeDocument * newClassifierCodeDocument (UMLClassifier * classifier);

    // return "Ruby"
    Uml::Programming_Language getLanguage();

    /**
     * Convert a C++ type such as 'int' or 'QWidget' to
     * ruby types Integer and Qt::Widget
     * @param cppType the C++ type to be converted
     */
    static QString cppToRubyType(const QString &cppType);

    /**
     * Convert C++ names such as 'm_foobar' or pFoobar to
     * just 'foobar' for ruby
     * @param cppName the C++ name to be converted
     */
    static QString cppToRubyName(const QString &cppName);

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

protected:

    /**
     * Create the codeblock that will represent the class declaration
     * for this classifier
     */
    CodeBlockWithComments * createClassDecl ( UMLClassifier *c, RubyClassifierCodeDocument * doc);

};

#endif // RUBYCODEGENERATOR_H

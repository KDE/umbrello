/***************************************************************************
                          rubycodegenerator.cpp
                          Derived from the Java code generator by thomas

    begin                : Thur Jul 21 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYCODEGENERATOR_H
#define RUBYCODEGENERATOR_H

#include <qstring.h>
#include "../codeviewerstate.h"
#include "../codegenerator.h"
#include "../codeblockwithcomments.h"
#include "../umldoc.h"

#include "classifierinfo.h"
#include "rubyclassifiercodedocument.h"

#include "rubycodegenerationpolicy.h"

class CodeViewerDialog;

class RubyCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:

    // Constructors/Destructors
    //

    /**
     * Empty Constructor
     */
    RubyCodeGenerator ();
    RubyCodeGenerator (QDomElement & element);

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeGenerator ( );

    // Public attributes
    //


    // Public attribute accessor methods
    //

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

    /** Get the editing dialog for this code document
     */
    virtual CodeViewerDialog * getCodeViewerDialog( QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState state);

    // Other methods
    //

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
     *
     * @param cppType the C++ type to be converted
     */
    static QString cppToRubyType(const QString &cppType);

    /**
     * Convert C++ names such as 'm_foobar' or pFoobar to
     * just 'foobar' for ruby
     *
     * @param cppName the C++ name to be converted
     */
    static QString cppToRubyName(const QString &cppName);

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

protected:

    /** create the codeblock that will represent the class declaration
     *  for this classifier
     */
    CodeBlockWithComments * createClassDecl ( UMLClassifier *c, ClassifierInfo *info, RubyClassifierCodeDocument * doc);

};

#endif // RUBYCODEGENERATOR_H

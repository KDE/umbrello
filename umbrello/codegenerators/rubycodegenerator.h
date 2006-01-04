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
           * A utility method to get the rubyCodeGenerationPolicy()->getCommentStyle() value.
            */
    RubyCodeGenerationPolicy::RubyCommentStyle getCommentStyle ( );

    /**
           * A utility method to get the rubyCodeGenerationPolicy()->getAutoGenerateConstructors() value.
     */
    bool getAutoGenerateConstructors ( );

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
    QString getListFieldClassName();

    /** Get the editing dialog for this code document
     */
    virtual CodeViewerDialog * getCodeViewerDialog( QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState state);

    // Other methods
    //

    // general purpose function we may reuse for all types of Ruby code documents
    QString capitalizeFirstLetter(QString string);

    /**
     * Utility function for getting the ruby code generation policy.
     */
    RubyCodeGenerationPolicy * getRubyPolicy();

    /**
     * @return    ClassifierCodeDocument
     * @param    classifier 
     */
    CodeDocument * newClassifierCodeDocument (UMLClassifier * classifier);

    // returns a ruby code generation policy
    virtual CodeGenerationPolicy * newCodeGenerationPolicy(KConfig * config = 0);

    CodeComment * newCodeComment ( CodeDocument * doc) ;

    // return "Ruby"
    Uml::Programming_Language getLanguage();

    /**
     * Convert a C++ type such as 'int' or 'QWidget' to 
     * ruby types Integer and Qt::Widget
     *
     * @param cppType the C++ type to be converted
     */
    QString cppToRubyType(QString cppType);

    /**
     * Convert C++ names such as 'm_foobar' or pFoobar to 
     * just 'foobar' for ruby
     *
     * @param cppName the C++ name to be converted
     */
    QString cppToRubyName(QString cppName);

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

protected:

    /** create the codeblock that will represent the class declaration
            *  for this classifier
         */
    CodeBlockWithComments * createClassDecl ( UMLClassifier *c, ClassifierInfo *info, RubyClassifierCodeDocument * doc);


    /**
     *
     */
    void setRubyPolicy( RubyCodeGenerationPolicy * policy);

    /**
     *
     */
    void setPolicy( CodeGenerationPolicy * policy);

private:


    // Make it easier on ourselves, utility function to get the ruby code
    // generation policy by simply storing in explict field (rather than in
    // CodeGenerationPolicy *)
    RubyCodeGenerationPolicy * m_rubycodegenerationpolicy;

    void initFields( ) ;
};

#endif // RUBYCODEGENERATOR_H

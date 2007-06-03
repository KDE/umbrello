
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DCODEGENERATOR_H
#define DCODEGENERATOR_H

#include <qstring.h>
#include "../codeviewerstate.h"
#include "../codegenerator.h"
#include "../codeblockwithcomments.h"
#include "../umldoc.h"

#include "classifierinfo.h"
#include "dclassifiercodedocument.h"

#include "dcodegenerationpolicy.h"

class CodeViewerDialog;

class DCodeGenerator : public CodeGenerator
{
    Q_OBJECT
public:

    /**
     * Empty Constructor
     */
    DCodeGenerator ();
    DCodeGenerator (QDomElement & element);

    /**
     * Empty Destructor
     */
    virtual ~DCodeGenerator ( );

    // Public attributes
    //


    // Public attribute accessor methods
    //

    /**
     * A utility method to get the dCodeGenerationPolicy()->getAutoGenerateAttribAccessors() value.
     */
    bool getAutoGenerateAttribAccessors( );

    /**
     * A utility method to get the dCodeGenerationPolicy()->getAutoGenerateAssocAccessors() value.
     */
    bool getAutoGenerateAssocAccessors( );

    /**
     * Get the list variable class name to use. For D, we have set this to "Vector".
     */
    static QString getListFieldClassName();

    /** Get the editing dialog for this code document
     */
    virtual CodeViewerDialog * getCodeViewerDialog( QWidget* parent, CodeDocument * doc,
            Settings::CodeViewerState state);

    // Other methods
    //

    // general purpose function we may reuse for all types of D code documents
    static QString capitalizeFirstLetter(const QString &string);

    /**
     * Utility function for getting the d code generation policy.
     */
    DCodeGenerationPolicy * getDPolicy();

    /**
     * @return  ClassifierCodeDocument
     * @param   classifier
     */
    CodeDocument * newClassifierCodeDocument (UMLClassifier * classifier);

    // return "D"
    Uml::Programming_Language getLanguage();

    /**
     * Adds D's primitives as datatypes
     */
    virtual QStringList defaultDatatypes();

    /**
     * IF the type is "string" we need to declare it as
     * the D Object "String" (there is no string primative in D).
     * Same thing again for "bool" to "boolean".
     */
    static QString fixTypeName(const QString &string);

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

protected:

    /** create the codeblock that will represent the class declaration
     *  for this classifier
     */
    CodeBlockWithComments * createClassDecl ( UMLClassifier *c, ClassifierInfo *info, DClassifierCodeDocument * doc);

private:

    void init();
};

#endif // DCODEGENERATOR_H

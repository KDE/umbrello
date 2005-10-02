/***************************************************************************
                          rubyclassdeclarationblock.cpp
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


#ifndef RUBYCLASSIFIERCODEDOCUMENT_H
#define RUBYCLASSIFIERCODEDOCUMENT_H

#include <qstring.h>
#include "../codeclassfieldlist.h"
#include "../classifiercodedocument.h"
#include "../classifier.h"
#include "../hierarchicalcodeblock.h"
#include "classifierinfo.h"
#include "rubycodeclassfield.h"
#include "rubycodeoperation.h"

class RubyCodeGenerator;
class RubyClassDeclarationBlock;
class RubyCodeGenerationPolicy;

/**
  * class RubyClassifierCodeDocument
  * A Ruby UMLClassifier Code Document.
  */

class RubyClassifierCodeDocument : public ClassifierCodeDocument
{
    Q_OBJECT
public:

    // Constructors/Destructors
    //


    /**
     * Constructor
     */
    RubyClassifierCodeDocument (UMLClassifier * classifier , RubyCodeGenerator * parent);

    /**
     * Empty Destructor
     */
    virtual ~RubyClassifierCodeDocument ( );

    /**
     * Get the dialog widget which allows user interaction with the object parameters.
     * @return    CodeDocumentDialog
     */
    //CodeDocumentDialog getDialog ( );

    QString scopeToRubyDecl(Uml::Visibility scope);
    QString capitalizeFirstLetter(const QString &string);

    // Make it easier on ourselves
    RubyCodeGenerationPolicy * getRubyPolicy();

    QString getRubyClassName (const QString &name);

    QString getPath();

    /** add a code operation to this ruby classifier code document.
     *  @return bool which is true IF the code operation was added successfully
     */
    bool addCodeOperation (CodeOperation * op );

    /**
            * create a new CodeClassField declaration block object belonging to this CodeDocument.
     */
    virtual CodeClassFieldDeclarationBlock * newDeclarationCodeBlock (CodeClassField * cf);

    /**
      * create a new CodeAccesorMethod object belonging to this CodeDocument.
      * @return      CodeAccessorMethod
      */
    virtual CodeAccessorMethod * newCodeAccessorMethod( CodeClassField *cf, CodeAccessorMethod::AccessorType type );

    /**
      * create a new CodeOperation object belonging to this CodeDocument.
      * @return      CodeOperation
      */
    virtual CodeOperation * newCodeOperation( UMLOperation * op );

    /**
     * create a new code comment. IN this case it is a RubyCodeComment.
     */
    virtual CodeComment * newCodeComment ( );

protected:

    // reset/clear our inventory of textblocks in this document
    void resetTextBlocks();

    /**
      * need to overwrite this for ruby since we need to pick up the 
    * ruby class declaration block.
      */
    virtual void loadChildTextBlocksFromNode ( QDomElement & root);

    /** create new code classfield for this document.
     */
    virtual CodeClassField * newCodeClassField( UMLAttribute *at);
    virtual CodeClassField * newCodeClassField( UMLRole *role);

    void addOrUpdateCodeClassFieldMethodsInCodeBlock(CodeClassFieldList &list, RubyClassDeclarationBlock * codeBlock);

    // IF the classifier object is modified, this will get called.
    // Possible mods include changing the filename and package
    // based on values the classifier has.
    virtual void syncNamesToParent( );

    bool forceDoc ();

    void updateContent();

private:

    RubyClassDeclarationBlock * classDeclCodeBlock;
    HierarchicalCodeBlock * constructorBlock;
    HierarchicalCodeBlock * operationsBlock;

    HierarchicalCodeBlock * publicBlock;
    HierarchicalCodeBlock * privateBlock;
    HierarchicalCodeBlock * protectedBlock;

//    HierarchicalCodeBlock * namespaceBlock;

    HierarchicalCodeBlock * pubConstructorBlock;
    HierarchicalCodeBlock * protConstructorBlock;
    HierarchicalCodeBlock * privConstructorBlock;

    HierarchicalCodeBlock * pubOperationsBlock;
    HierarchicalCodeBlock * privOperationsBlock;
    HierarchicalCodeBlock * protOperationsBlock;

    ClassifierInfo * info;
    QString fileName; // Just for our convience in creating code
    QString endLine; // characters for ending line. Just for our convience in creating code
    QString RubyClassName;

    void init ( );
    RubyClassDeclarationBlock * getClassDecl();


};

#endif // RUBYCLASSIFIERCODEDOCUMENT_H

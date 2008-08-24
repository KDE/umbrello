/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/


#ifndef RUBYCLASSIFIERCODEDOCUMENT_H
#define RUBYCLASSIFIERCODEDOCUMENT_H

#include "codeclassfieldlist.h"
#include "classifiercodedocument.h"
#include "classifier.h"
#include "hierarchicalcodeblock.h"
#include "rubycodeclassfield.h"
#include "rubycodeoperation.h"

#include <QtCore/QString>

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

    /**
     * Constructor
     */
    RubyClassifierCodeDocument (UMLClassifier * classifier);

    /**
     * Empty Destructor
     */
    virtual ~RubyClassifierCodeDocument ( );

    /**
     * Get the dialog widget which allows user interaction with the object parameters.
     * @return    CodeDocumentDialog
     */
    //CodeDocumentDialog getDialog ( );

    // Make it easier on ourselves
    RubyCodeGenerationPolicy * getRubyPolicy();

    QString getRubyClassName (const QString &name);

    QString getPath();

    /**
     * Add a code operation to this ruby classifier code document.
     * @return bool which is true IF the code operation was added successfully
     */
    bool addCodeOperation (CodeOperation * op );

    void updateContent();

protected:

    // reset/clear our inventory of textblocks in this document
    void resetTextBlocks();

    /**
     * Need to overwrite this for ruby since we need to pick up the
     * ruby class declaration block.
     */
    virtual void loadChildTextBlocksFromNode ( QDomElement & root);

    void addOrUpdateCodeClassFieldMethodsInCodeBlock(CodeClassFieldList &list, RubyClassDeclarationBlock * codeBlock);

    bool forceDoc ();

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

    void init ( );
    RubyClassDeclarationBlock * getClassDecl();

};

#endif // RUBYCLASSIFIERCODEDOCUMENT_H

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DCLASSIFIERCODEDOCUMENT_H
#define DCLASSIFIERCODEDOCUMENT_H

#include <QtCore/QString>

#include "codeclassfieldlist.h"
#include "classifiercodedocument.h"
#include "classifier.h"
#include "hierarchicalcodeblock.h"
#include "dcodeclassfield.h"
#include "dcodeoperation.h"

class DClassDeclarationBlock;
class DCodeGenerationPolicy;

/**
  * class DClassifierCodeDocument
  * A D UMLClassifier Code Document.
  */

class DClassifierCodeDocument : public ClassifierCodeDocument
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    DClassifierCodeDocument (UMLClassifier * classifier);

    /**
     * Empty Destructor
     */
    virtual ~DClassifierCodeDocument ( );

//    /**
//     * Get the dialog widget which allows user interaction with the object parameters.
//     * @return  CodeDocumentDialog
//     */
//    CodeDocumentDialog getDialog ( );

    // Make it easier on ourselves
    DCodeGenerationPolicy * getDPolicy();

    QString getDClassName (const QString &name);

    QString getPath();

    /**
     * Add a code operation to this d classifier code document.
     * @return bool which is true IF the code operation was added successfully
     */
    bool addCodeOperation (CodeOperation * op );

    void updateContent();

protected:

    // reset/clear our inventory of textblocks in this document
    void resetTextBlocks();

    /**
     * Need to overwrite this for d since we need to pick up the
     * d class declaration block.
     */
    virtual void loadChildTextBlocksFromNode ( QDomElement & root);

    void addOrUpdateCodeClassFieldMethodsInCodeBlock(CodeClassFieldList &list, DClassDeclarationBlock * codeBlock);

    bool forceDoc ();

private:

    DClassDeclarationBlock * classDeclCodeBlock;
    HierarchicalCodeBlock * constructorBlock;
    HierarchicalCodeBlock * operationsBlock;

/*
    QString fileName; // Just for our convience in creating code
    QString endLine; // characters for ending line. Just for our convience in creating code
    QString DClassName;
*/
    void init ( );
    DClassDeclarationBlock * getClassDecl();


};

#endif // DCLASSIFIERCODEDOCUMENT_H

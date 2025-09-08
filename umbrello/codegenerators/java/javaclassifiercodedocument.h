/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACLASSIFIERCODEDOCUMENT_H
#define JAVACLASSIFIERCODEDOCUMENT_H

#include "codeclassfieldlist.h"
#include "classifiercodedocument.h"
#include "umlclassifier.h"
#include "hierarchicalcodeblock.h"
#include "javacodeclassfield.h"
#include "javacodeoperation.h"

#include <QString>

class JavaClassDeclarationBlock;
class JavaCodeGenerationPolicy;

/**
  * class JavaClassifierCodeDocument
  * A Java UMLClassifier Code Document.
  */

class JavaClassifierCodeDocument : public ClassifierCodeDocument
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    explicit JavaClassifierCodeDocument (UMLClassifier * classifier);

    /**
     * Empty Destructor
     */
    virtual ~JavaClassifierCodeDocument ();

    /**
     * Get the dialog widget which allows user interaction with the object parameters.
     * @return  CodeDocumentDialog
     */
    //CodeDocumentDialog getDialog ();

    // Make it easier on ourselves
    JavaCodeGenerationPolicy * getJavaPolicy() const;

    QString getJavaClassName (const QString &name) const;

    QString getPath() const;

    /**
     * Add a code operation to this java classifier code document.
     * @return bool which is true IF the code operation was added successfully
     */
    bool addCodeOperation (CodeOperation * op);

    void updateContent();

protected:

    // reset/clear our inventory of textblocks in this document
    void resetTextBlocks();

    /**
     * Need to overwrite this for java since we need to pick up the
     * java class declaration block.
     */
    virtual void loadChildTextBlocksFromNode (QDomElement & root);

    void addOrUpdateCodeClassFieldMethodsInCodeBlock(CodeClassFieldList &list, JavaClassDeclarationBlock * codeBlock);

    bool forceDoc ();

private:

    JavaClassDeclarationBlock * classDeclCodeBlock;
    HierarchicalCodeBlock * constructorBlock;
    HierarchicalCodeBlock * operationsBlock;

    void init ();
    JavaClassDeclarationBlock * getClassDecl();
};

#endif // JAVACLASSIFIERCODEDOCUMENT_H

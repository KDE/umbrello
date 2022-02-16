/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPHEADERCODEDOCUMENT_H
#define CPPHEADERCODEDOCUMENT_H

#include "codeclassfieldlist.h"
#include "classifiercodedocument.h"
#include "hierarchicalcodeblock.h"

#include <QString>

class CPPHeaderClassDeclarationBlock;

/**
 * class CPPHeaderCodeDocument
 * A CPP UMLClassifier Header Code Document.
 * We carve the CPP document up into 2 documents, "source" and "header".
 * This one represents the header portion.
 * The sections of each are as follows:
 * - header
 * - includes
 * - import statements
 * - class declaration
 * -   guts of the class (e.g. field decl, accessor methods, operations, dependent classes)
 */
class CPPHeaderCodeDocument : public ClassifierCodeDocument
{
    Q_OBJECT
public:

    explicit CPPHeaderCodeDocument(UMLClassifier* classifier);
    virtual ~CPPHeaderCodeDocument();

    bool addCodeOperation(CodeOperation* op);

    void updateContent();

    /**
     * Save the XMI representation of this object
     */
    //virtual void saveToXMI(QXmlStreamWriter& writer);

protected:

    // reset/clear our inventory of textblocks in this document
    void resetTextBlocks();

    /**
     * Need to overwrite this for cpp header since we need to pick up the
     * header class declaration block.
     */
    virtual void loadChildTextBlocksFromNode(QDomElement & root);

    void addOrUpdateCodeClassFieldMethodsInCodeBlock(CodeClassFieldList &list, CPPHeaderClassDeclarationBlock * codeBlock);

    /**
     * Create a new code comment. IN this case it is a CPPCodeDocumentation object.
     */
    CodeComment * newCodeComment();

private:

    CPPHeaderClassDeclarationBlock * m_classDeclCodeBlock;

    HierarchicalCodeBlock * m_publicBlock;
    HierarchicalCodeBlock * m_privateBlock;
    HierarchicalCodeBlock * m_protectedBlock;

    HierarchicalCodeBlock * m_namespaceBlock;

    HierarchicalCodeBlock * m_pubConstructorBlock;
    HierarchicalCodeBlock * m_protConstructorBlock;
    HierarchicalCodeBlock * m_privConstructorBlock;

    HierarchicalCodeBlock * m_pubOperationsBlock;
    HierarchicalCodeBlock * m_privOperationsBlock;
    HierarchicalCodeBlock * m_protOperationsBlock;

    CPPHeaderClassDeclarationBlock * getClassDecl();

};

#endif // CPPHEADERCODEDOCUMENT_H

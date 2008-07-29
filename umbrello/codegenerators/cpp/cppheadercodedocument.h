/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CPPHEADERCODEDOCUMENT_H
#define CPPHEADERCODEDOCUMENT_H

#include "codeclassfieldlist.h"
#include "classifiercodedocument.h"
#include "hierarchicalcodeblock.h"

#include <QtCore/QString>

class CPPHeaderClassDeclarationBlock;

/**
 * class CPPHeaderCodeDocument
 * A CPP UMLClassifier Header Code Document.
 */
class CPPHeaderCodeDocument : public ClassifierCodeDocument
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    CPPHeaderCodeDocument (UMLClassifier * classifier);

    /**
     * Empty Destructor
     */
    virtual ~CPPHeaderCodeDocument ( );

    /** add a code operation to this cpp classifier code document.
     *  @return bool which is true IF the code operation was added successfully
     */
    bool addCodeOperation (CodeOperation * op );

    void updateContent();

    /**
     * Save the XMI representation of this object
     */
    //virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

protected:

    // reset/clear our inventory of textblocks in this document
    void resetTextBlocks();

    /**
     * Need to overwrite this for cpp header since we need to pick up the
     * header class declaration block.
     */
    virtual void loadChildTextBlocksFromNode ( QDomElement & root);

    void addOrUpdateCodeClassFieldMethodsInCodeBlock(CodeClassFieldList &list, CPPHeaderClassDeclarationBlock * codeBlock);

    /**
     * Create a new code comment. IN this case it is a CPPCodeDocumentation object.
     */
    CodeComment * newCodeComment ( );

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

    /**
     *
     */
    CPPHeaderClassDeclarationBlock * getClassDecl();

};

#endif // CPPHEADERCODEDOCUMENT_H

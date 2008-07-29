/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CPPSOURCECODEDOCUMENT_H
#define CPPSOURCECODEDOCUMENT_H

#include "classifiercodedocument.h"
#include "hierarchicalcodeblock.h"

#include <QtCore/QString>

/**
 * A CPP UMLClassifier Source Code Document.
 */
class CPPSourceCodeDocument : public ClassifierCodeDocument
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    CPPSourceCodeDocument(UMLClassifier * classifier);

    /**
     * Empty Destructor
     */
    virtual ~CPPSourceCodeDocument();

    /**
     * Add a code operation to this cpp classifier code document.
     * In the vannilla version, we just tack all operations on the end
     * of the document.
     * @param op   the code operation
     * @return     bool which is true IF the code operation was added successfully
     */
    bool addCodeOperation(CodeOperation * op);

    /**
     * This method will cause the class to rebuild its text representation.
     * based on the parent classifier object.
     * For any situation in which this is called, we are either building the code
     * document up, or replacing/regenerating the existing auto-generated parts. As
     * such, we will want to insert everything we reasonably will want
     * during creation. We can set various parts of the document (esp. the
     * comments) to appear or not, as needed.
     */
    void updateContent();

protected:

    /**
     * Reset/clear our inventory of textblocks in this document.
     */
    void resetTextBlocks();

private:

    HierarchicalCodeBlock * m_constructorBlock;
    HierarchicalCodeBlock * m_methodsBlock;

};

#endif // CPPSOURCECODEDOCUMENT_H

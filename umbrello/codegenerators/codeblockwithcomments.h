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

#ifndef CODEBLOCKWITHCOMMENTS_H
#define CODEBLOCKWITHCOMMENTS_H

#include "codeblock.h"
#include "codecomment.h"

class HierarchicalCodeBlock;

/**
 * class CodeBlockWithComments
 * A very common type of text block in any type of code.
 */
class CodeBlockWithComments : public CodeBlock
{
public:

    explicit CodeBlockWithComments ( CodeDocument * parent , const QString & body = "", const QString & comment = "");

    /**
     * Empty Destructor
     */
    virtual ~CodeBlockWithComments ( );

    void setComment ( CodeComment * object );

    CodeComment * getComment ( ) const;

    virtual QString toString ( ) const;

    void setOverallIndentationLevel ( int level );

    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    virtual void loadFromXMI ( QDomElement & root );

    virtual void setAttributesFromObject (TextBlock * obj);

protected:

    virtual void setAttributesOnNode (QDomDocument & doc, QDomElement & blockElement);

    virtual void setAttributesFromNode ( QDomElement & element);

private:

    CodeComment * m_comment;

};

#endif // CODEBLOCKWITHCOMMENTS_H

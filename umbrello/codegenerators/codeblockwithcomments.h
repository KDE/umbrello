/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CODEBLOCKWITHCOMMENTS_H
#define CODEBLOCKWITHCOMMENTS_H

#include "codeblock.h"
#include "codecomment.h"


/**
 * class CodeBlockWithComments
 * A very common type of text block in any type of code.
 */
class CodeBlockWithComments : public CodeBlock
{
public:

    explicit CodeBlockWithComments (CodeDocument * parent, const QString & body = QString(), const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~CodeBlockWithComments ();

    void setComment (CodeComment * object);

    CodeComment * getComment () const;

    virtual QString toString () const;

    void setOverallIndentationLevel (int level);

    virtual void saveToXMI1 (QDomDocument & doc, QDomElement & root);

    virtual void loadFromXMI1 (QDomElement & root);

    virtual void setAttributesFromObject (TextBlock * obj);

protected:

    virtual void setAttributesOnNode (QDomDocument & doc, QDomElement & blockElement);

    virtual void setAttributesFromNode (QDomElement & element);

private:

    CodeComment * m_comment;

};

#endif // CODEBLOCKWITHCOMMENTS_H

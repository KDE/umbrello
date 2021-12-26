/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual void loadFromXMI (QDomElement & root);

    virtual void setAttributesFromObject (TextBlock * obj);

protected:

    virtual void setAttributesOnNode (QXmlStreamWriter& writer);

    virtual void setAttributesFromNode (QDomElement & element);

private:

    CodeComment * m_comment;

};

#endif // CODEBLOCKWITHCOMMENTS_H

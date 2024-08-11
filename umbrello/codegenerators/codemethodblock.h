/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEMETHODBLOCK_H
#define CODEMETHODBLOCK_H

#include "basictypes.h"
#include "ownedcodeblock.h"
#include "codeblockwithcomments.h"

#include <QString>

class ClassifierCodeDocument;
class QXmlStreamWriter;

/**
 * class CodeMethodBlock
 * A  common type of "code block" that occurs in OO code.
 * Note: keep the inheritance sequence: QObject needs to be first in inheritance list.
 */
class CodeMethodBlock : public OwnedCodeBlock, public CodeBlockWithComments
{
    friend class CodeGenObjectWithTextBlocks;
    Q_OBJECT
public:

    /**
     * Constructors
     */
    CodeMethodBlock (ClassifierCodeDocument * doc, UMLObject * parentObj,
                      const QString & body = QString(), const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~CodeMethodBlock ();

    virtual QString toString () const;

    QString getStartMethodText () const;

    QString getEndMethodText () const;

    CodeDocument * getParentDocument();

    /**
     * This is the method called from within syncToparent()
     * to update the *body* of the method.
     * It is only called if the method is Auto-generated.
     */
    virtual void updateContent () = 0;

protected:
    virtual void release ();

    void setStartMethodText (const QString &value);

    void setEndMethodText (const QString &value);

    /**
     * This is the method called from within syncToparent().
     * To update the start and end Method text. It is called
     * whether or not the method is Auto or User generated.
     */
    virtual void updateMethodDeclaration () = 0;

    virtual void setAttributesOnNode (QXmlStreamWriter& writer);

    virtual void setAttributesFromNode (QDomElement & element);

    virtual void setAttributesFromObject (TextBlock * obj);

private:
    QString m_startMethod;
    QString m_endMethod;

public Q_SLOTS:
    virtual void syncToParent();
};

#endif // CODEMETHODBLOCK_H

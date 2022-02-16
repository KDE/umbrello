/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef HIERARCHICALCODEBLOCK_H
#define HIERARCHICALCODEBLOCK_H

#include "codegenobjectwithtextblocks.h"
#include "codeblockwithcomments.h"
#include "codeclassfieldlist.h"

#include <QMap>
#include <QString>

class QXmlStreamWriter;

class HierarchicalCodeBlock : public CodeBlockWithComments, public CodeGenObjectWithTextBlocks
{
    friend class CodeGenObjectWithTextBlocks;
public:

    explicit HierarchicalCodeBlock (CodeDocument * doc, const QString &startString = QString(), const QString &endString = QString(), const QString &comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~HierarchicalCodeBlock ();

    void setEndText (const QString &new_var);

    QString getEndText () const;

    bool addTextBlock (TextBlock * add_object);

    bool insertTextBlock (TextBlock * newBlock, TextBlock * existingBlock, bool after = true);

    bool removeTextBlock (TextBlock * remove_object);

    void setStartText (const QString &text);

    QString getStartText () const;

    virtual void saveToXMI(QXmlStreamWriter& writer);

    virtual void loadFromXMI (QDomElement & root);

    virtual QString toString () const;

    QString getUniqueTag();
    QString getUniqueTag(const QString& prefix);

    void addCodeClassFieldMethods (CodeClassFieldList &list);

    virtual CodeBlock * newCodeBlock();
    virtual CodeBlockWithComments * newCodeBlockWithComments();
    virtual HierarchicalCodeBlock * newHierarchicalCodeBlock();

protected:

    virtual void release ();

    virtual void setAttributesOnNode (QXmlStreamWriter& writer);

    virtual void setAttributesFromNode (QDomElement & element);

    virtual void setAttributesFromObject (TextBlock * obj);

    TextBlock * findCodeClassFieldTextBlockByTag (const QString &tag);

private:

    QString m_startText;
    QString m_endText;

    QString childTextBlocksToString() const;
    void initAttributes () ;

};

#endif // HIERARCHICALCODEBLOCK_H

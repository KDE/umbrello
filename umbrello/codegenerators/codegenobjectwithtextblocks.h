/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODEGENOBJECTWITHTEXTBLOCKS_H
#define CODEGENOBJECTWITHTEXTBLOCKS_H

#include "codeaccessormethod.h"
#include "textblocklist.h"

#include <QMap>

class CodeBlock;
class CodeBlockWithComments;
class CodeClassField;
class CodeComment;
class CodeDocument;
class HierarchicalCodeBlock;
class QXmlStreamWriter;
class TextBlock;

/**
 * This abstract class is for code generator objects which 'own' text blocks.
 */
class CodeGenObjectWithTextBlocks
{
public:

    explicit CodeGenObjectWithTextBlocks(CodeDocument *parent);
    virtual ~CodeGenObjectWithTextBlocks();

    virtual bool addTextBlock(TextBlock * add_object);

    virtual bool removeTextBlock(TextBlock * remove_object);

    /**
     * Insert a new text block before/after the existing text block. Returns
     * false if it cannot insert the textblock.
     */
    virtual bool insertTextBlock(TextBlock * newBlock, TextBlock * existingBlock, bool after) = 0;

    TextBlockList * getTextBlockList() const;

    virtual HierarchicalCodeBlock * getHierarchicalCodeBlock(const QString &tag, const QString &comment, int indentLevel);

    virtual CodeBlockWithComments * getCodeBlockWithComments(const QString &tag, const QString &comment, int indentLevel);

    CodeComment * addOrUpdateTaggedCodeComment(const QString &tag = QString(), const QString &text = QString(), int indentationLevel = 0);

    CodeBlockWithComments * addOrUpdateTaggedCodeBlockWithComments
                             (const QString &tag, const QString &text, const QString &ctext,
                              int indentLevel, bool forceUserBlockUpdate);

    TextBlock * findTextBlockByTag(const QString &tag);

    /**
     * @return  QString
     * @param   prefix
     */
    virtual QString getUniqueTag(const QString& prefix = QString()) = 0;

    /**
     * Virtual methods that return a new code document objects.
     */
    virtual CodeBlock * newCodeBlock() = 0;
    virtual CodeBlockWithComments * newCodeBlockWithComments() = 0;
    virtual HierarchicalCodeBlock * newHierarchicalCodeBlock() = 0;

    CodeGenObjectWithTextBlocks * findParentObjectForTaggedTextBlock(const QString & tag);

protected:

    virtual void setAttributesOnNode(QXmlStreamWriter& writer);

    virtual void setAttributesFromNode(QDomElement & element);

    /*virtual*/ void setAttributesFromObject(CodeGenObjectWithTextBlocks * obj);

    virtual void loadChildTextBlocksFromNode(QDomElement & root);

    virtual void resetTextBlocks();

    /**
     * Find specific text block belonging to code classfields.
     * Block may not presently be allocated to the textblock list.
     */
    virtual TextBlock * findCodeClassFieldTextBlockByTag(const QString &tag) = 0;

    QMap<QString, TextBlock *> m_textBlockTagMap;
    TextBlockList m_textblockVector;

private:

    // needed in order to use findTextBlocksByTag
    CodeDocument *m_pCodeDoc;

};

#endif // CODEGENOBJECTWITHTEXTBLOCKS_H

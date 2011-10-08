/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TEXTBLOCK_H
#define TEXTBLOCK_H

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtCore/QObject>

class CodeDocument;

/**
 * The fundemental unit of text within an output file containing code.
 */
class TextBlock
{
    friend class CodeGenObjectWithTextBlocks;
    friend class ClassifierCodeDocument;
public:

    explicit TextBlock(CodeDocument * parent, const QString & text = "");
    virtual ~TextBlock();

    void setText(const QString & text);
    QString getText() const;
    void appendText(const QString & text);

    void setTag(const QString & value);
    QString getTag() const;

    CodeDocument * getParentDocument() const;

    void setWriteOutText(bool write);
    bool getWriteOutText() const;

    void setIndentationLevel(int level);
    int getIndentationLevel() const;

    QString getIndentationString(int level = 0) const;

    static QString getIndentation();

    static QString getNewLineEndingChars();

    static QString formatMultiLineText(const QString & work, const QString & linePrefix,
                                  const QString & breakStr,
                                  bool addBreak = true, bool lastLineHasBreak = true);

    virtual QString unformatText(const QString & text, const QString & indent = "");

    virtual QString toString() const;

    static QString encodeText(const QString & text, const QString & endLine);
    static QString decodeText(const QString & text, const QString & endLine);

    /**
     * Save the XMI representation of this object
     * @param doc    the xmi document
     * @param root   the starting point to append
     */
    virtual void saveToXMI(QDomDocument & doc, QDomElement & root) = 0;

    /**
     * Load params from the appropriate XMI element node.
     * @param root   the starting point in the xmi document to load from
     */
    virtual void loadFromXMI(QDomElement & root) = 0;

    bool canDelete() const;

    virtual void setAttributesFromObject(TextBlock * obj);

    virtual QString getNewEditorLine(int amount = 0);

    virtual int firstEditableLine();
    virtual int lastEditableLine();

    friend QDebug operator<<(QDebug os, const TextBlock& obj);

protected:

    void setCanDelete(bool canDelete);

    virtual void release();

    virtual void setAttributesOnNode(QDomDocument & doc, QDomElement & blockElement);
    virtual void setAttributesFromNode(QDomElement & root);

private:

    QString m_text;   //< The actual text of this code block.
    QString m_tag;
    bool    m_canDelete;
    bool    m_writeOutText;   //< Flag to write the text of this TextBlock into a file.
    int     m_indentationLevel;
    CodeDocument * m_parentDocument;

};

#endif // TEXTBLOCK_H

/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codeblockwithcomments.h"

// local includes
#include "codedocument.h"
#include "codegenfactory.h"
#include "umlapp.h"

// qt/kde includes
#include <QXmlStreamWriter>

/**
 * Basic Constructor
 */
CodeBlockWithComments::CodeBlockWithComments (CodeDocument * parent, const QString & body, const QString & comment)
        : CodeBlock (parent, body)
{
    CodeComment * codecomment = CodeGenFactory::newCodeComment(parent);
    codecomment->setText(comment);
    m_comment = codecomment;
}

CodeBlockWithComments::~CodeBlockWithComments ()
{
    delete m_comment;
}

/**
 * Set the Comment object.
 */
void CodeBlockWithComments::setComment (CodeComment * object)
{
    m_comment = object;
}

/**
 * Get the Comment object.
 */
CodeComment * CodeBlockWithComments::getComment () const
{
    return m_comment;
}

/**
 * Save the XMI representation of this object
 */
void CodeBlockWithComments::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("codeblockwithcomments"));

    // set attributes
    setAttributesOnNode(writer);

    writer.writeEndElement();
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeBlockWithComments::setAttributesOnNode (QXmlStreamWriter& writer)
{
    // set super-class attributes
    CodeBlock::setAttributesOnNode(writer);

    // set local attributes now..e.g. a comment
    // which we will store in its own separate child node block
    writer.writeStartElement(QStringLiteral("header"));
    getComment()->saveToXMI(writer); // comment
    writer.writeEndElement();
}

/**
 * Set the class attributes from a passed object.
 */
void CodeBlockWithComments::setAttributesFromObject(TextBlock * obj)
{
    CodeBlock::setAttributesFromObject(obj);

    CodeBlockWithComments * cb = dynamic_cast<CodeBlockWithComments*>(obj);
    if (cb) {
        getComment()->setAttributesFromObject((TextBlock*)cb->getComment());
    }
}

/**
 * Load params from the appropriate XMI element node.
 */
void CodeBlockWithComments::loadFromXMI (QDomElement & root)
{
    setAttributesFromNode(root);
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void CodeBlockWithComments::setAttributesFromNode(QDomElement & root)
{
    // set attributes from superclass method the XMI
    CodeBlock::setAttributesFromNode(root);

    // load comment now
    // by looking for our particular child element
    QDomNode node = root.firstChild();
    QDomElement element = node.toElement();
    bool gotComment = false;
    while (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QStringLiteral("header")) {
            QDomNode cnode = element.firstChild();
            QDomElement celem = cnode.toElement();
            getComment()->loadFromXMI(celem);
            gotComment = true;
            break;
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    if (!gotComment) {
        logWarn1("CodeBlockWithComments::loadFromXMI : unable to initialize CodeComment in block %1", getTag());
    }
}

/**
 * @return  QString
 */
QString CodeBlockWithComments::toString () const
{
    QString string;

    if (getWriteOutText()) {
        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        QString body = formatMultiLineText (getText(), indent, endLine);
        CodeComment* codeComment = getComment();
        QString comment = codeComment->toString();

        if (!comment.isEmpty() && codeComment->getWriteOutText()) {
            string.append(comment);
        }
        if (!body.isEmpty()) {
            string.append(body);
        }
    }
    return string;
}

// slave indentation level for both the header and text body

/**
 * A utility method that causes the comment and body of the code block
 * to have the same indentation level.
 */
void CodeBlockWithComments::setOverallIndentationLevel (int level)
{
    setIndentationLevel(level);
    getComment()->setIndentationLevel(level);
}

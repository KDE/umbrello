/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "codeblock.h"

#include "codedocument.h"
#include "debug_utils.h"

#include <QTextStream>

/**
 * Constructor.
 * @param doc    the documentation text
 * @param body   the body text
 * */
CodeBlock::CodeBlock(CodeDocument * doc, const QString & body)
  : TextBlock(doc, body),
    m_contentType(AutoGenerated)
{
}

/**
 * Empty Destructor.
 */
CodeBlock::~CodeBlock()
{
}

/**
 * Set the value of m_contentType
 * specifies whether the content (text) of this object was generated by the code
 * generator or was supplied by the user.
 * @param new_var the new value of m_contentType
 */
void CodeBlock::setContentType(ContentType new_var)
{
    m_contentType = new_var;
}

/**
 * Get the value of m_contentType
 * specifies whether the content (text) of this object was generated by the code
 * generator or was supplied by the user.
 * @return the value of m_contentType
 */
CodeBlock::ContentType CodeBlock::contentType() const
{
    return m_contentType;
}

/**
 * Save the XMI representation of this object.
 * @param doc    the xmi document
 * @param root   the starting point to append
 */
void CodeBlock::saveToXMI1(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("codeblock"));
    // set attributes
    setAttributesOnNode(writer);
    writer.writeEndElement();
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 * @param doc            the xmi document
 * @param blockElement   the xmi element holding the attributes
 */
void CodeBlock::setAttributesOnNode(QXmlStreamWriter& writer)
{
    // call super-class
    TextBlock::setAttributesOnNode(writer);
    // local attributes
    if (m_contentType != AutoGenerated)
        writer.writeAttribute(QLatin1String("contentType"), QString::number(contentType()));
}

/**
 * Load params from the appropriate XMI element node.
 * @param root   the starting point to load from
 */
void CodeBlock::loadFromXMI1(QDomElement & root)
{
    setAttributesFromNode(root);
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 * @param elem   the xmi element from which to load
 */
void CodeBlock::setAttributesFromNode(QDomElement & elem)
{
    // set attributes from the XMI in super-class
    TextBlock::setAttributesFromNode(elem);
    // set local fields now
    setContentType(((ContentType) elem.attribute(QLatin1String("contentType"), QLatin1String("0")).toInt()));
}

/**
 * Set the class attributes from a passed object.
 * @param obj   text block from which the attributes are taken
 */
void CodeBlock::setAttributesFromObject(TextBlock * obj)
{
    TextBlock::setAttributesFromObject(obj);
    CodeBlock * cb = dynamic_cast<CodeBlock*>(obj);
    if (cb)
        setContentType(cb->contentType());
}

/**
 * Return a string representation of ContentType.
 *
 * @param  val  the enum value of the ContentType
 * @return      the string representation of the enum
 */
QString CodeBlock::enumToString(const ContentType& val)
{
    if (val == AutoGenerated) {
        return QLatin1String("AutoGenerated");
    }
    else {
        return QLatin1String("UserGenerated");
    }
}

QDebug operator<<(QDebug str, const CodeBlock& obj)
{
    str.nospace() << "CodeBlock: " << CodeBlock::enumToString(obj.contentType())
        << ", ..." << static_cast<TextBlock*>(const_cast<CodeBlock*>(&obj));
    return str.space();
}

/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "xmlelementcodeblock.h"

// local includes
#include "attribute.h"
#include "codedocument.h"
#include "debug_utils.h"
#include "xmlcodecomment.h"

// qt includes
#include <QXmlStreamWriter>

XMLElementCodeBlock::XMLElementCodeBlock (CodeDocument * parentDoc, const QString & nodeName, const QString & comment)
        : HierarchicalCodeBlock(parentDoc)
{
    init(parentDoc, nodeName, comment);
}

XMLElementCodeBlock::~XMLElementCodeBlock ()
{
}

/**
 * Save the XMI representation of this object
 */
void XMLElementCodeBlock::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("xmlelementblock"));

    setAttributesOnNode(writer);

    writer.writeEndElement();
}

/**
 * load params from the appropriate XMI element node.
 */
void XMLElementCodeBlock::loadFromXMI (QDomElement & root)
{
    setAttributesFromNode(root);
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void XMLElementCodeBlock::setAttributesOnNode (QXmlStreamWriter& writer)
{
    // superclass call
    HierarchicalCodeBlock::setAttributesOnNode(writer);

    // now set local attributes/fields
    writer.writeAttribute(QLatin1String("nodeName"), getNodeName());
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void XMLElementCodeBlock::setAttributesFromNode (QDomElement & root)
{
    // superclass call
    HierarchicalCodeBlock::setAttributesFromNode(root);

    // now set local attributes
    setNodeName(root.attribute(QLatin1String("nodeName"), QLatin1String("UNKNOWN")));
}

void XMLElementCodeBlock::setNodeName (const QString &name)
{
    m_nodeName = name;
}

QString XMLElementCodeBlock::getNodeName ()
{
    return m_nodeName;
}

void XMLElementCodeBlock::addAttribute (UMLAttribute * at)
{
    m_attList.append(at);
}

UMLAttributeList * XMLElementCodeBlock::getAttributeList()
{
    return & m_attList;
}

/**
 * Update the start and end text for this ownedhierarchicalcodeblock.
 */
void XMLElementCodeBlock::updateContent ()
{
    QString endLine = getNewLineEndingChars();

    QString nodeName = getNodeName();

    // Now update START/ENDING Text
    QString startText = QLatin1Char('<') + nodeName;
    QString endText;

    UMLAttributeList * alist = getAttributeList();
    foreach (UMLAttribute *at, *alist)
    {
        if(at->getInitialValue().isEmpty())
            uWarning() << " XMLElementCodeBlock : cant print out attribute that lacks an initial value";
        else {
            startText.append(QLatin1String(" ") + at->name() + QLatin1String("=\""));
            startText.append(at->getInitialValue() + QLatin1String("\""));
        }
    }

    // now set close of starting/ending node, the style depending on whether we have child text or not
    if(getTextBlockList()->count())
    {
        startText.append(QLatin1String(">"));
        endText = QLatin1String("</") + nodeName + QLatin1Char('>');
    } else {
        startText.append(QLatin1String("/>"));
        endText = QString();
    }

    setStartText(startText);
    setEndText(endText);
}

void XMLElementCodeBlock::init (CodeDocument *parentDoc, const QString &nodeName, const QString &comment)
{
    setComment(new XMLCodeComment(parentDoc));
    getComment()->setText(comment);

    m_nodeName = nodeName;

    updateContent();
}

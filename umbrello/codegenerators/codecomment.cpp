/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "codecomment.h"

#include "codedocument.h"

CodeComment::CodeComment(CodeDocument * doc, const QString & comment)
  : TextBlock(doc, comment)
{
}

/**
 * Empty Destructor.
 */
CodeComment::~CodeComment()
{
}

/**
 * Save the XMI representation of this object.
 */
void CodeComment::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("codecomment"));
    setAttributesOnNode(writer); // as we added no additional fields to this class we may
    // just use parent TextBlock method
    writer.writeEndElement();
}

/**
 * Load params from the appropriate XMI element node.
 */
void CodeComment::loadFromXMI(QDomElement & root)
{
    setAttributesFromNode(root);
}

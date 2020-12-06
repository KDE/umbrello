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
void CodeComment::saveToXMI1(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("codecomment"));
    setAttributesOnNode(writer); // as we added no additional fields to this class we may
    // just use parent TextBlock method
    writer.writeEndElement();
}

/**
 * Load params from the appropriate XMI element node.
 */
void CodeComment::loadFromXMI1(QDomElement & root)
{
    setAttributesFromNode(root);
}

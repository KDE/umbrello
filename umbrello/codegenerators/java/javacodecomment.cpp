/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "javacodecomment.h"

#include <QtCore/QRegExp>

JavaCodeComment::JavaCodeComment ( CodeDocument * doc, const QString & text )
        : CodeComment (doc, text)
{
}

JavaCodeComment::~JavaCodeComment ( )
{
}

QString JavaCodeComment::getNewEditorLine ( int amount )
{
    QString line = getIndentationString(amount) + "// ";
    return line;
}

/** UnFormat a long text string. Typically, this means removing
 *  the indentaion (linePrefix) and/or newline chars from each line.
 */
QString JavaCodeComment::unformatText ( const QString & text , const QString & indent)
{
    // remove leading or trailing comment stuff
    QString mytext = TextBlock::unformatText(text, indent);

    // now leading slashes
    mytext.remove(QRegExp("^\\/\\/\\s*"));
    return mytext;
}

/**
 * @return      QString
 */
QString JavaCodeComment::toString ( ) const
{
    QString output = "";

    // simple output method
    if (getWriteOutText())
    {
        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        output.append(formatMultiLineText (getText(), indent +"// ", endLine));
    }

    return output;
}

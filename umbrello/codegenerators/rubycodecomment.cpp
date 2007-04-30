/***************************************************************************
                          rubycodecomment.cpp
                          Derived from the Java code generator by thomas

    begin                : Thur Jul 21 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "rubycodecomment.h"
#include <qregexp.h>

// Constructors/Destructors
//

RubyCodeComment::RubyCodeComment(CodeDocument * doc, const QString & text)
        : CodeComment(doc, text)
{

}

RubyCodeComment::~RubyCodeComment( ) { }

//
// Methods
//


// Accessor methods
//


// Public attribute accessor methods
//

// Other methods
//

QString RubyCodeComment::getNewEditorLine( int amount ) {
    QString line = getIndentationString(amount) + "# ";
    return line;
}

/** UnFormat a long text string. Typically, this means removing
 *  the indentaion (linePrefix) and/or newline chars from each line.
 */
QString RubyCodeComment::unformatText(const QString & text, const QString & indent)
{
    // remove leading or trailing comment stuff
    QString mytext = TextBlock::unformatText(text, indent);

    // now leading hash
    mytext.remove(QRegExp("^#\\s*"));
    return mytext;
}

/**
 * @return    QString
 */
QString RubyCodeComment::toString( )
{
    QString output = "";

    // simple output method
    if (getWriteOutText()) {
        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        output.append(formatMultiLineText(getText(), indent + "# ", endLine + endLine));
    }

    return output;
}


#include "rubycodecomment.moc"

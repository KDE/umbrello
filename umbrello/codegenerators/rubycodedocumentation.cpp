/***************************************************************************
                          rubycodedocumentation.cpp
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
 ***************************************************************************/


#include <kdebug.h>

#include <qregexp.h>

#include "rubycodedocumentation.h"
#include "rubyclassifiercodedocument.h"
#include "rubycodegenerationpolicy.h"

// Constructors/Destructors
//

RubyCodeDocumentation::RubyCodeDocumentation ( RubyClassifierCodeDocument * doc, const QString & text )
        : CodeComment ((CodeDocument*) doc, text)
{

}

RubyCodeDocumentation::~RubyCodeDocumentation ( ) { }

//
// Methods
//


// Accessor methods
//

// Other methods
//

/**
 * Save the XMI representation of this object
 */
void RubyCodeDocumentation::saveToXMI ( QDomDocument & doc, QDomElement & root ) {
    QDomElement blockElement = doc.createElement( "rubycodedocumentation" );
    setAttributesOnNode(doc, blockElement); // as we added no additional fields to this class we may
    // just use parent TextBlock method
    root.appendChild( blockElement );
}

/**
 * @return    QString
 */
QString RubyCodeDocumentation::toString ( )
{

    QString output = "";

    // simple output method
    if(getWriteOutText())
    {
        bool useDoubleDashOutput = true;

        // need to figure out output type from ruby policy
        RubyCodeGenerationPolicy * p = ((RubyClassifierCodeDocument*)getParentDocument())->getRubyPolicy();
        if(p->getCommentStyle() == RubyCodeGenerationPolicy::BeginEnd)
            useDoubleDashOutput = false;

        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        QString body = getText();
        if(useDoubleDashOutput)
        {
            if(!body.isEmpty())
                output.append(formatMultiLineText (body, indent +"# ", endLine));
        } else {
            output.append("=begin rdoc"+endLine);
            output.append(formatMultiLineText (body, indent +" ", endLine));
            output.append("=end"+endLine);
        }
    }

    return output;
}

QString RubyCodeDocumentation::getNewEditorLine ( int amount )
{
    RubyCodeGenerationPolicy * p = ((RubyClassifierCodeDocument*)getParentDocument())->getRubyPolicy();
    if(p->getCommentStyle() == RubyCodeGenerationPolicy::BeginEnd)
        return getIndentationString(amount) + " ";
    else
        return getIndentationString(amount) + "# ";
}

int RubyCodeDocumentation::firstEditableLine() {
    RubyCodeGenerationPolicy * p = ((RubyClassifierCodeDocument*)getParentDocument())->getRubyPolicy();
    if(p->getCommentStyle() == RubyCodeGenerationPolicy::BeginEnd)
        return 1;
    return 0;
}

int RubyCodeDocumentation::lastEditableLine() {
    RubyCodeGenerationPolicy * p = ((RubyClassifierCodeDocument*)getParentDocument())->getRubyPolicy();
    if(p->getCommentStyle() == RubyCodeGenerationPolicy::BeginEnd)
    {
        return -1; // very last line is NOT editable
    }
    return 0;
}

/** UnFormat a long text string. Typically, this means removing
 *  the indentaion (linePrefix) and/or newline chars from each line.
 */
QString RubyCodeDocumentation::unformatText ( const QString & text , const QString & indent)
{

    QString mytext = TextBlock::unformatText(text, indent);
    RubyCodeGenerationPolicy * p = ((RubyClassifierCodeDocument*)getParentDocument())->getRubyPolicy();
    // remove leading or trailing comment stuff
    mytext.remove(QRegExp("^"+indent));
    if(p->getCommentStyle() == RubyCodeGenerationPolicy::BeginEnd)
    {
        mytext.remove(QRegExp("^=begin\\s*(rdoc)?\\s*\n?"));
        mytext.remove(QRegExp("^=end\\s*\n?$"));
    } else
        mytext.remove(QRegExp("^#\\s*"));

    return mytext;
}


#include "rubycodedocumentation.moc"

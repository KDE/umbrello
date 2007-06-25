/***************************************************************************
                          rubycodecomment.h
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

#ifndef RUBYCODECOMMENT_H
#define RUBYCODECOMMENT_H

#include <qstring.h>
#include "../codecomment.h"

/**
  * class RubyCodeComment
  * A Ruby code comment. There are two styles of comments:
  * these are simply started with a hash and no terminating
  * characters, or delimited by '=begin' and '=end' tags at the
  * start of lines
  */

class RubyCodeComment : virtual public CodeComment
{
    Q_OBJECT
public:
    /**
     * Constructors
     */
    explicit RubyCodeComment ( CodeDocument * doc, const QString & text = "");

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeComment ( );

    /**
     * @return    QString
     */
    QString toString ( );

    /** UnFormat a long text string. Typically, this means removing
      *  the indentaion (linePrefix) and/or newline chars from each line.
      */
    virtual QString unformatText ( const QString & text, const QString & indent = "" );

    /** a special version here because we want to not only indent
     * the new line, but to add the "#" as well.
     */
    virtual QString getNewEditorLine ( int amount );

protected:

private:

};

#endif // RUBYCODECOMMENT_H

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DCODECOMMENT_H
#define DCODECOMMENT_H

#include "codecomment.h"

#include <QtCore/QString>

/**
  * A class representing a D code comment.
  *
  * The syntax follows the D Style Guide (@see http://www.digitalmars.com/d/dstyle.html).
  * Single line comments begin with // and multiline comments use the longer form.
  */
class DCodeComment : virtual public CodeComment
{
public:

    /**
     * Constructors
     */
    explicit DCodeComment ( CodeDocument * doc, const QString & text = "");

    /**
     * Empty Destructor
     */
    virtual ~DCodeComment ( );

    /**
     * Save the XMI representation of this object.
     */
    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    /**
     * Creates a comment block.
     *
     * The syntax follows the D Style Guide (@see http://www.digitalmars.com/d/dstyle.html).
     * Single line comments begin with // and multiline comments use the longer form.
     *
     * @return  QString Comment block source code.
     */
    QString toString ( ) const;

    /**
     * UnFormat a long text string. Typically, this means removing
     * the indentaion (linePrefix) and/or newline chars from each line.
     */
    virtual QString unformatText ( const QString & text, const QString & indent = "" );

    /**
     * A special version here because we want to not only indent
     * the new line, but to add the "//" sequence as well.
     */
    virtual QString getNewEditorLine ( int amount );
};

#endif // DCODECOMMENT_H

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

#ifndef JAVACODEDOCUMENTATION_H
#define JAVACODEDOCUMENTATION_H

#include "codecomment.h"

#include <QtCore/QString>

class JavaClassifierCodeDocument;

/**
 * class JavaCodeDocumentation
 * A Java code comment. There is only a single styles of comments:
 * these are simply started with double slash sequence and no terminating
 * characters
 */
class JavaCodeDocumentation : virtual public CodeComment
{
public:

    /**
     * Constructors
     */
    explicit JavaCodeDocumentation ( JavaClassifierCodeDocument * doc, const QString & text = "");

    /**
     * Empty Destructor
     */
    virtual ~JavaCodeDocumentation ( );

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    /**
     * @return  QString
     */
    QString toString ( ) const;

    /**
    * UnFormat a long text string. Typically, this means removing
    * the indentaion (linePrefix) and/or newline chars from each line.
    */
    virtual QString unformatText ( const QString & text, const QString & indent = "" );

    /**
     * A special version here because we want to not only indent
     * the new line, but to add the " * " sequence as well.
     */
    virtual QString getNewEditorLine ( int amount );

    /**
     * Ush. These are terrifically bad and must one day go away.
     * Both methods indicate the range of lines in this textblock
     * which may be edited by the codeeditor (assuming that any are
     * actually editable). The default case is no lines are editable.
     * The line numbering starts with '0' and a '-1' means no line
     * qualifies.
     */
    virtual int firstEditableLine();
    virtual int lastEditableLine();


protected:

private:

};

#endif // JAVACODEDOCUMENTATION_H

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

#ifndef DCODEDOCUMENTATION_H
#define DCODEDOCUMENTATION_H

#include "codecomment.h"

#include <QtCore/QString>

class CodeDocument;

/**
 * A class representing a D documentation comment.
 *
 * The syntax follows the D Style Guide (@see http://www.digitalmars.com/d/dstyle.html).
 * Choice between single line and multiline comments is based on the common policy.
 */
class DCodeDocumentation : virtual public CodeComment
{
public:

    /**
     * Constructors
     */
    explicit DCodeDocumentation ( CodeDocument * doc, const QString & text = "");

    /**
     * Empty Destructor
     */
    virtual ~DCodeDocumentation ( );

    /**
     * Save the XMI representation of this object.
     */
    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    /**
     * Creates a documentation block.
     *
     * The syntax follows the D Style Guide (@see http://www.digitalmars.com/d/dstyle.html).
     * Choice between single line and multiline comments is based on the common policy.
     *
     * @return  QString Documentation block source code.
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
     * which may be edited by the code editor (assuming that any are
     * actually editable). The default case is no lines are editable.
     * The line numbering starts with '0' and a '-1' means no line
     * qualifies.
     */
    virtual int firstEditableLine();
    virtual int lastEditableLine();

};

#endif // DCODEDOCUMENTATION_H

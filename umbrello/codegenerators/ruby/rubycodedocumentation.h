/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef RUBYCODEDOCUMENTATION_H
#define RUBYCODEDOCUMENTATION_H

#include "codecomment.h"

#include <QString>

class RubyClassifierCodeDocument;

/**
 * class RubyCodeDocumentation
 * A Ruby code comment. There is only a single styles of comments:
 * these are simply started with a hash and no terminating
 * characters
 */

class RubyCodeDocumentation : virtual public CodeComment
{
public:

    /**
     * Constructors
     */
    explicit RubyCodeDocumentation (RubyClassifierCodeDocument * doc, const QString & text = QString());

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeDocumentation ();

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI1(QXmlStreamWriter& writer);

    /**
     * @return    QString
     */
    QString toString () const;

    /**
     * UnFormat a long text string. Typically, this means removing
     * the indentation (linePrefix) and/or newline chars from each line.
     */
    virtual QString unformatText (const QString & text, const QString & indent = QString());

    /**
     * A special version here because we want to not only indent
     * the new line, but to add the "# " sequence as well.
     */
    virtual QString getNewEditorLine (int amount);

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

#endif // RUBYCODEDOCUMENTATION_H

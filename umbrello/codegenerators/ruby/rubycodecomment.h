/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef RUBYCODECOMMENT_H
#define RUBYCODECOMMENT_H

#include "codecomment.h"

#include <QString>

/**
 * class RubyCodeComment
 * A Ruby code comment. There are two styles of comments:
 * these are simply started with a hash and no terminating
 * characters, or delimited by '=begin' and '=end' tags at the
 * start of lines
 */
class RubyCodeComment : virtual public CodeComment
{
public:
    /**
     * Constructors
     */
    explicit RubyCodeComment (CodeDocument * doc, const QString & text = QString());

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeComment ();

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
     * the new line, but to add the "#" as well.
     */
    virtual QString getNewEditorLine (int amount);

protected:

private:

};

#endif // RUBYCODECOMMENT_H

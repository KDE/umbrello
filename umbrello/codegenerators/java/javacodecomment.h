/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACODECOMMENT_H
#define JAVACODECOMMENT_H

#include "codecomment.h"

#include <QString>

/**
 * class JavaCodeComment
 * A Java code comment. There is only a single styles of comments:
 * these are simply started with double slash sequence and no terminating
 * characters
 */
class JavaCodeComment : public CodeComment
{
public:

    /**
     * Constructors
     */
    explicit JavaCodeComment (CodeDocument * doc, const QString & text = QString());

    /**
     * Empty Destructor
     */
    virtual ~JavaCodeComment ();

    /**
     * @return  QString
     */
    QString toString () const;

    /**
     * UnFormat a long text string. Typically, this means removing
     *  the indentation (linePrefix) and/or newline chars from each line.
     */
    virtual QString unformatText (const QString & text, const QString & indent = QString());

    /**
     * A special version here because we want to not only indent
     * the new line, but to add the "//" sequence as well.
     */
    virtual QString getNewEditorLine (int amount);

protected:

private:

};

#endif // JAVACODECOMMENT_H

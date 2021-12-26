/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPCODECOMMENT_H
#define CPPCODECOMMENT_H

#include "codecomment.h"

#include <QString>

/**
 * A CPP code comment. There is only a single styles of comments:
 * these are simply started with double slash sequence and no terminating
 * characters.
 */
class CPPCodeComment : virtual public CodeComment
{
public:

    /**
     * Constructors.
     */
    explicit CPPCodeComment (CodeDocument * doc, const QString & text = QString());

    /**
     * Empty Destructor.
     */
    virtual ~CPPCodeComment ();

    /**
     * Save the XMI representation of this object.
     */
    virtual void saveToXMI(QXmlStreamWriter& writer);

    /**
     * @return  QString
     */
    QString toString () const;

    /**
     * UnFormat a long text string. Typically, this means removing
     * the indentation (linePrefix) and/or newline chars from each line.
     */
    virtual QString unformatText (const QString & text, const QString & indent = QString());

    /**
     * A special version here because we want to not only indent
     * the new line, but to add the "//" sequence as well.
     */
    virtual QString getNewEditorLine (int amount);

};

#endif // CPPCODECOMMENT_H

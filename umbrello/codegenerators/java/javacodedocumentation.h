/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACODEDOCUMENTATION_H
#define JAVACODEDOCUMENTATION_H

#include "codecomment.h"

#include <QString>

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
    explicit JavaCodeDocumentation (JavaClassifierCodeDocument * doc, const QString & text = QString());

    /**
     * Empty Destructor
     */
    virtual ~JavaCodeDocumentation ();

    /**
     * Save the XMI representation of this object
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
     * the new line, but to add the " * " sequence as well.
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

#endif // JAVACODEDOCUMENTATION_H

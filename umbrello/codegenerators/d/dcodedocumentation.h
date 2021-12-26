/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCODEDOCUMENTATION_H
#define DCODEDOCUMENTATION_H

#include "codecomment.h"

#include <QString>

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
    explicit DCodeDocumentation (CodeDocument * doc, const QString & text = QString());

    /**
     * Empty Destructor
     */
    virtual ~DCodeDocumentation ();

    /**
     * Save the XMI representation of this object.
     */
    virtual void saveToXMI(QXmlStreamWriter& writer);

    /**
     * Creates a documentation block.
     *
     * The syntax follows the D Style Guide (@see http://www.digitalmars.com/d/dstyle.html).
     * Choice between single line and multiline comments is based on the common policy.
     *
     * @return  QString Documentation block source code.
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
     * which may be edited by the code editor (assuming that any are
     * actually editable). The default case is no lines are editable.
     * The line numbering starts with '0' and a '-1' means no line
     * qualifies.
     */
    virtual int firstEditableLine();
    virtual int lastEditableLine();

};

#endif // DCODEDOCUMENTATION_H

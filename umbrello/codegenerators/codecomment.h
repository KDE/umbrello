/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    copyright (C) 2004-2020
    Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CODECOMMENT_H
#define CODECOMMENT_H

#include "textblock.h"

#include <QString>

/**
 * Text which will be comments. These should be bracketed by what ever code type
 * comment the language requires.
 */
class CodeComment : public TextBlock
{
public:

    explicit CodeComment(CodeDocument * doc, const QString & comment = QString());
    virtual ~CodeComment();

    virtual void saveToXMI1(QXmlStreamWriter& writer);
    virtual void loadFromXMI1(QDomElement & root);

protected:

private:

};

#endif // CODECOMMENT_H

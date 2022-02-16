/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef XMLCODECOMMENT_H
#define XMLCODECOMMENT_H

#include "codecomment.h"

#include <QString>

class CodeDocument;

/**
 * class XMLCodeDocumentation
 * A XML (code) comment.
 */
class XMLCodeComment: virtual public CodeComment
{
public:

    /**
     * Constructors
     */
    explicit XMLCodeComment (CodeDocument * doc, const QString & text = QString());

    /**
     * Empty Destructor
     */
    virtual ~XMLCodeComment();

    /**
     * @return  QString
     */
    QString toString () const;

protected:

private:

};

#endif // XMLCODECOMMENT_H

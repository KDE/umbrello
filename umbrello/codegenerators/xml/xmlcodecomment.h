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

#ifndef XMLCODECOMMENT_H
#define XMLCODECOMMENT_H

#include "codecomment.h"

#include <QtCore/QString>

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
    explicit XMLCodeComment ( CodeDocument * doc, const QString & text = "" );

    /**
     * Empty Destructor
     */
    virtual ~XMLCodeComment( );

    /**
     * @return  QString
     */
    QString toString ( ) const;

protected:

private:

};

#endif // XMLCODECOMMENT_H

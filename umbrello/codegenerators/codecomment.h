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

#ifndef CODECOMMENT_H
#define CODECOMMENT_H

#include "textblock.h"

#include <QtCore/QString>

/**
 * Text which will be comments. These should be bracketed by what ever code type
 * comment the language requires.
 */
class CodeComment : public TextBlock
{
public:

    explicit CodeComment ( CodeDocument * doc, const QString & comment = "");

    /**
     * Empty Destructor.
     */
    virtual ~CodeComment ( );

    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    virtual void loadFromXMI ( QDomElement & root );

protected:

private:

};

#endif // CODECOMMENT_H

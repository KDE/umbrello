/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2012                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "umlscene.h"

#include <QtDebug>

QDebug operator<<(QDebug out, UMLScene *item)
{
    out.nospace() << "UMLScene: " << static_cast<UMLView *>(item);
    return out;
}


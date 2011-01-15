/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLENTITYLIST_H
#define UMLENTITYLIST_H

#include <QtCore/QList>

// forward declaration
class UMLEntity;

typedef QList<UMLEntity*> UMLEntityList;
typedef QListIterator<UMLEntity*> UMLEntityListIt;

#endif

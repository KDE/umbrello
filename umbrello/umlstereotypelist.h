/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2010                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLSTEREOTYPELIST_H
#define UMLSTEREOTYPELIST_H

#include <QtCore/QList>

// forward declaration
class UMLStereotype;

typedef QList<UMLStereotype*> UMLStereotypeList;
typedef QListIterator<UMLStereotype*> UMLStereotypeListIt;

#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLASSOCIATIONLIST_H
#define UMLASSOCIATIONLIST_H

#include <qlist.h>

// forward declaration
class UMLAssociation;

typedef QList<UMLAssociation*> UMLAssociationList;
typedef QListIterator<UMLAssociation*> UMLAssociationListIt;

#endif

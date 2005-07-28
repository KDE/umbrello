/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLASSOCIATIONLIST_H
#define UMLASSOCIATIONLIST_H

#include <q3ptrlist.h>

// forward declaration
class UMLAssociation;

typedef Q3PtrList<UMLAssociation> UMLAssociationList;
typedef Q3PtrListIterator<UMLAssociation> UMLAssociationListIt;

#endif

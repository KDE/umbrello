/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLDATATYPELIST_H
#define UMLDATATYPELIST_H

#include <qptrlist.h>

// forward declaration
class UMLDatatype;

typedef QPtrList<UMLDatatype> UMLDatatypeList;
typedef QPtrListIterator<UMLDatatype> UMLDatatypeListIt;

#endif

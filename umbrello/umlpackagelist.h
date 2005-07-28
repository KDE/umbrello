/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLPACKAGELIST_H
#define UMLPACKAGELIST_H

#include <q3ptrlist.h>

class UMLPackage;

typedef Q3PtrList<UMLPackage> UMLPackageList;
typedef Q3PtrListIterator<UMLPackage> UMLPackageListIt;

#endif

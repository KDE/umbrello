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

#include <qptrlist.h>

class UMLPackage;

typedef QPtrList<UMLPackage> UMLPackageList;
typedef QPtrListIterator<UMLPackage> UMLPackageListIt;

#endif

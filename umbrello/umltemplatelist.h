/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLTEMPLATELIST_H
#define UMLTEMPLATELIST_H

#include <qptrlist.h>

// forward declaration
class UMLTemplate;

typedef QPtrList<UMLTemplate> UMLTemplateList;
typedef QPtrListIterator<UMLTemplate> UMLTemplateListIt;

#endif

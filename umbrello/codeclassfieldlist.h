/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CODECLASSFIELDLIST_H
#define _CODECLASSFIELDLIST_H

#include <qptrlist.h>

// forward declarations
class CodeClassField;

typedef QPtrList<CodeClassField> CodeClassFieldList;
typedef QPtrListIterator<CodeClassField> CodeClassFieldListIt;

#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CODEDOCUMENTLIST_H
#define _CODEDOCUMENTLIST_H

#include <q3ptrlist.h>

// forward declarations
class CodeDocument;

typedef Q3PtrList<CodeDocument> CodeDocumentList;
typedef Q3PtrListIterator<CodeDocument> CodeDocumentListIt;

#endif

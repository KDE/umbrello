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

#include <qptrlist.h>

// forward declarations
class CodeDocument;

typedef QPtrList<CodeDocument> CodeDocumentList;
typedef QPtrListIterator<CodeDocument> CodeDocumentListIt;

#endif

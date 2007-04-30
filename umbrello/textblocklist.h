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

#ifndef _TEXTBLOCKLIST_H
#define _TEXTBLOCKLIST_H

#include <qptrlist.h>

// forward declarations
class TextBlock;

typedef QPtrList<TextBlock> TextBlockList;
typedef QPtrListIterator<TextBlock> TextBlockListIt;

#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MESSAGEWIDGETLIST_H
#define MESSAGEWIDGETLIST_H

#include <q3ptrlist.h>

class MessageWidget;

typedef Q3PtrList<MessageWidget> MessageWidgetList;
typedef Q3PtrListIterator<MessageWidget> MessageWidgetListIt;

#endif

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

#include <qptrlist.h>

class MessageWidget;

typedef QPtrList<MessageWidget> MessageWidgetList;
typedef QPtrListIterator<MessageWidget> MessageWidgetListIt;

#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef MESSAGEWIDGETLIST_H
#define MESSAGEWIDGETLIST_H

#include <QList>
#include <QPointer>

class MessageWidget;

typedef QList<QPointer<MessageWidget>> MessageWidgetList;
typedef QListIterator<QPointer<MessageWidget>> MessageWidgetListIt;

#endif

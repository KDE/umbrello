/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2001      Gustavo Madrigal <gmadrigal@nextphere.com>    *
 *   copyright (C) 2004-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef UMLWIDGETLIST_H
#define UMLWIDGETLIST_H

#include <QList>
#include <QPointer>
class UMLWidget;

typedef QList<QPointer<UMLWidget>> UMLWidgetList;
typedef QListIterator<QPointer<UMLWidget>> UMLWidgetListIt;

#endif

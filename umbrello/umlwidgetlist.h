/***************************************************************************
                          umlwidgetlist.h  -  description
                             -------------------
    begin                : Sat Dec 29 2001
    copyright            : (C) 2001 by Gustavo Madrigal
    email                : gmadrigal@nextphere.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef UMLWIDGETLIST_H
#define UMLWIDGETLIST_H

#include <qptrlist.h>

class UMLWidget;
class UMLWidgetData;

typedef QPtrList<UMLWidget> UMLWidgetList;
typedef QPtrListIterator<UMLWidget> UMLWidgetListIt;

typedef QPtrList<UMLWidgetData> UMLWidgetDataList;
typedef QPtrListIterator<UMLWidgetData> UMLWidgetDataListIt;

#endif

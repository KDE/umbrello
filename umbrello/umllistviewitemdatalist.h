/***************************************************************************
                          umllistviewitemdatalist.h  -  description
                             -------------------
    begin                : Sun Jan 13 2002
    copyright            : (C) 2002 by Gustavo Madrigal
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

#ifndef UMLLISTVIEWITEMDATALIST_H
#define UMLLISTVIEWITEMDATALIST_H

#include "umllistviewitemdata.h"
#include <qlist.h>

typedef QList<UMLListViewItemData> UMLListViewItemDataList;
typedef QListIterator<UMLListViewItemData> UMLListViewItemDataListIt;

#endif

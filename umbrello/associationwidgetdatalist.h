/***************************************************************************
                          associationwidgetdatalist.h  -  description
                             -------------------
    begin                : Thu Mar 28
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

#ifndef _ASSOCIATIONWIDGETDATALIST_H
#define _ASSOCIATIONWIDGETDATALIST_H

#include "associationwidgetdata.h"
#include <qlist.h>

typedef QList<AssociationWidgetData> AssociationWidgetDataList;
typedef QListIterator<AssociationWidgetData> AssociationWidgetDataListIt;
typedef QList<AssociationWidget> AssociationWidgetList;
typedef QListIterator<AssociationWidget> AssociationWidgetListIt;

#endif

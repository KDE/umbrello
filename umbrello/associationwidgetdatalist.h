/***************************************************************************
                          associationwidgetdatalist.h  -  description
                             -------------------
    begin                : Thu Mar 28
    copyright            : (C) 2002 by Gustavo Madrigal
    email                : gmadrigal@nextphere.com
  Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
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

#include <qptrlist.h>

// forward declarations
class AssociationWidgetData;

typedef QPtrList<AssociationWidgetData> AssociationWidgetDataList;
typedef QPtrListIterator<AssociationWidgetData> AssociationWidgetDataListIt;

#endif

/***************************************************************************
                          umlviewlist.h  -  description
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

#ifndef UMLVIEWLIST_H
#define UMLVIEWLIST_H

#include "umlviewdata.h"
#include <qptrlist.h>

typedef QPtrList<UMLView> UMLViewList;
typedef QPtrListIterator<UMLView> UMLViewListIt;

typedef QPtrList<UMLViewData> UMLViewDataList;
typedef QPtrListIterator<UMLViewData> UMLViewDataListIt;
#endif

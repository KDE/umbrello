/***************************************************************************
                          associationwidgetlist.h  -  description
                             -------------------
    copyright            : (C) 2003 by Oliver Kellogg
    email                : okellogg@users.sourceforge.net
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

#ifndef _ASSOCIATIONWIDGETLIST_H
#define _ASSOCIATIONWIDGETLIST_H

#include <qptrlist.h>

// forward declarations
class AssociationWidget;

typedef QPtrList<AssociationWidget> AssociationWidgetList;
typedef QPtrListIterator<AssociationWidget> AssociationWidgetListIt;

#endif

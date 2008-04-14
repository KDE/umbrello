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

#ifndef ASSOCIATIONWIDGETLIST_H
#define ASSOCIATIONWIDGETLIST_H

#include <QtCore/QList>

// forward declarations
class AssociationWidget;

typedef QList<AssociationWidget*> AssociationWidgetList;
typedef QListIterator<AssociationWidget*> AssociationWidgetListIt;

#endif

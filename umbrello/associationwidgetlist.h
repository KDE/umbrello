/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
    copyright            : (C) 2003 by Oliver Kellogg
    email                : okellogg@users.sourceforge.net
  Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 ***************************************************************************/

#ifndef ASSOCIATIONWIDGETLIST_H
#define ASSOCIATIONWIDGETLIST_H

#include <QList>
#include <QPointer>

// forward declarations
class AssociationWidget;

typedef QList<QPointer<AssociationWidget>> AssociationWidgetList;
typedef QListIterator<QPointer<AssociationWidget>> AssociationWidgetListIt;

#endif

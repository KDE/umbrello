/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Oliver Kellogg <okellogg@users.sourceforge.net>
    Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
*/

#ifndef ASSOCIATIONWIDGETLIST_H
#define ASSOCIATIONWIDGETLIST_H

#include <QList>
#include <QPointer>

// forward declarations
class AssociationWidget;

typedef QList<QPointer<AssociationWidget>> AssociationWidgetList;
typedef QListIterator<QPointer<AssociationWidget>> AssociationWidgetListIt;

#endif

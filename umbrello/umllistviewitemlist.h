/*
    SPDX-FileCopyrightText: 2001 Gustavo Madrigal < gmadrigal@nextphere.com>
    SPDX-License-Identifier: GPL-2.0-or-later
    Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
*/

#ifndef UMLLISTVIEWITEMLIST_H
#define UMLLISTVIEWITEMLIST_H

#include <QList>

class UMLListViewItem;

typedef QList<UMLListViewItem*> UMLListViewItemList;
typedef QListIterator<UMLListViewItem*> UMLListViewItemListIt;

#endif

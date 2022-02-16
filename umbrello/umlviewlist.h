/*
    SPDX-FileCopyrightText: 2001 Gustavo Madrigal <gmadrigal@nextphere.com>
    SPDX-License-Identifier: GPL-2.0-or-later
    Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
*/

#ifndef UMLVIEWLIST_H
#define UMLVIEWLIST_H

//#include "umlview.h"
#include <QList>
#include <QPointer>

class UMLView;

typedef QList<QPointer<UMLView>> UMLViewList;
typedef QListIterator<QPointer<UMLView>> UMLViewListIt;

#endif

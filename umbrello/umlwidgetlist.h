/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2001 Gustavo Madrigal <gmadrigal@nextphere.com>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef UMLWIDGETLIST_H
#define UMLWIDGETLIST_H

#include <QList>
#include <QPointer>
class UMLWidget;

typedef QList<QPointer<UMLWidget>> UMLWidgetList;
typedef QListIterator<QPointer<UMLWidget>> UMLWidgetListIt;

#endif

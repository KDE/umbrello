/*

    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef MESSAGEWIDGETLIST_H
#define MESSAGEWIDGETLIST_H

#include <QList>
#include <QPointer>

class MessageWidget;

typedef QList<QPointer<MessageWidget>> MessageWidgetList;
typedef QListIterator<QPointer<MessageWidget>> MessageWidgetListIt;

#endif

/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef WIDGETLIST_UTILS_H
#define WIDGETLIST_UTILS_H

#include "umlwidgetlist.h"

/**
 * General purpose widget list utilities.
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace WidgetList_Utils
{
    qreal getSmallestX(const UMLWidgetList &widgetList);
    qreal getSmallestY(const UMLWidgetList &widgetList);
    qreal getBiggestX(const UMLWidgetList &widgetList);
    qreal getBiggestY(const UMLWidgetList &widgetList);
    qreal getHeightsSum(const UMLWidgetList &widgetList);
    qreal getWidthsSum(const UMLWidgetList &widgetList);
}  // namespace WidgetList_Utils

#endif

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2009-2011                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef WIDGETLIST_UTILS_H
#define WIDGETLIST_UTILS_H

#include "umlwidgetlist.h"

/**
 * General purpose widget list utilities.
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
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

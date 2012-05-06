/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2009-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widgetlist_utils.h"

// app includes
#include "debug_utils.h"
#include "umlwidget.h"

// qt/kde includes
#include <QtCore/QVector>

namespace WidgetList_Utils
{

/**
 * Looks for the smallest x-value of the given UMLWidgets.
 * @param widgetList A list with UMLWidgets.
 */
qreal getSmallestX(const UMLWidgetList &widgetList)
{
    qreal smallestX = 0;

    int i = 1;
    foreach(UMLWidget *widget,  widgetList) {
        if (i == 1) {
            smallestX = widget->x();
        } else {
            if (smallestX > widget->x())
                smallestX = widget->x();
        }
        i++;
    }

    return smallestX;
}

/**
 * Looks for the smallest y-value of the given UMLWidgets.
 * @param widgetList A list with UMLWidgets.
 */
qreal getSmallestY(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    qreal smallestY = 0;

    int i = 1;
    foreach(UMLWidget *widget,  widgetList) {
        if (i == 1) {
            smallestY = widget->y();
        } else {
            if (smallestY > widget->y())
                smallestY = widget->y();
        }
        i++;
    }

    return smallestY;
}

/**
 * Looks for the biggest x-value of the given UMLWidgets.
 * @param widgetList A list with UMLWidgets.
 */
qreal getBiggestX(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    qreal biggestX = 0;

    int i = 1;
    foreach(UMLWidget *widget, widgetList) {
        if (i == 1) {
            biggestX = widget->x();
            biggestX += widget->width();
        } else {
            if (biggestX < widget->x() + widget->width())
                biggestX = widget->x() + widget->width();
        }
        i++;
    }

    return biggestX;
}

/**
 * Looks for the biggest y-value of the given UMLWidgets.
 * @param widgetList A list with UMLWidgets.
 */
qreal getBiggestY(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    qreal biggestY = 0;

    int i = 1;
    foreach(UMLWidget *widget, widgetList) {
        if (i == 1) {
            biggestY = widget->y();
            biggestY += widget->height();
        } else {
            if (biggestY < widget->y() + widget->height())
                biggestY = widget->y() + widget->height();
        }
        i++;
    }

    return biggestY;
}

/**
 * Returns the sum of the heights of the given UMLWidgets
 * @param widgetList A list with UMLWidgets.
 */
qreal getHeightsSum(const UMLWidgetList &widgetList)
{
    qreal heightsSum = 0;

    foreach(UMLWidget *widget, widgetList) {
        heightsSum += widget->height();
    }

    return heightsSum;
}

/**
 * Returns the sum of the widths of the given UMLWidgets.
 * @param widgetList A list with UMLWidgets.
 */
qreal getWidthsSum(const UMLWidgetList &widgetList)
{
    qreal widthsSum = 0;

    foreach(UMLWidget *widget, widgetList) {
        widthsSum += widget->width();
    }

    return widthsSum;
}

}  // namespace WidgetList_Utils

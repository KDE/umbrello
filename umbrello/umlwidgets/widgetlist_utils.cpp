/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2009-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "widgetlist_utils.h"

// app includes
#include "debug_utils.h"
#include "umlwidget.h"

// qt/kde includes
#include <QVector>

namespace WidgetList_Utils
{

/**
 * Looks for the smallest x-value of the given UMLWidgets.
 * @param widgetList A list with UMLWidgets.
 * @return The smallest X position.
 */
qreal getSmallestX(const UMLWidgetList &widgetList)
{
    qreal smallestX = 0;

    int i = 1;
    for(UMLWidget *widget : widgetList) {
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
 * @return The smallest Y position.
 */
qreal getSmallestY(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    qreal smallestY = 0;

    int i = 1;
    for(UMLWidget *widget : widgetList) {
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
 * @return The biggest X position.
 */
qreal getBiggestX(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    qreal biggestX = 0;

    int i = 1;
    Q_FOREACH(UMLWidget *widget, widgetList) {
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
 * @return The biggest Y position.
 */
qreal getBiggestY(const UMLWidgetList &widgetList)
{
    if (widgetList.isEmpty())
        return -1;

    qreal biggestY = 0;

    int i = 1;
    Q_FOREACH(UMLWidget *widget, widgetList) {
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

    Q_FOREACH(UMLWidget *widget, widgetList) {
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

    Q_FOREACH(UMLWidget *widget, widgetList) {
        widthsSum += widget->width();
    }

    return widthsSum;
}

}  // namespace WidgetList_Utils

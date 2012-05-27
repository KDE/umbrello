/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_utils.h"

// app includes
#include "debug_utils.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"

// qt includes
#include <QtGui/QBrush>
#include <QtGui/QPen>

namespace Widget_Utils
{

/**
 * Find the widget identified by the given ID in the given widget
 * or message list.
 *
 * @param id         The unique ID to find.
 * @param widgets    The UMLWidgetList to search in.
 * @param messages   Optional pointer to a MessageWidgetList to search in.
 */
UMLWidget* findWidget(Uml::IDType id,
                      const UMLWidgetList& widgets,
                      const MessageWidgetList* messages /* = 0 */)
{
    foreach (UMLWidget* obj, widgets) {
        if (obj->baseType() == WidgetBase::wt_Object) {
            if (static_cast<ObjectWidget *>(obj)->localID() == id)
                return obj;
        } else if (obj->id() == id) {
            return obj;
        }
    }

    if (messages) {
        foreach (UMLWidget* obj, *messages) {
            if (obj->id() == id)
                return obj;
        }
    }
    return 0;
}

/**
 * Creates the decoration point.
 * @param p   the base point
 * @return    the decoration point
 */
UMLSceneRectItem* decoratePoint(const UMLScenePoint& p)
{
    const int SIZE = 4;
    UMLView *currentView = UMLApp::app()->currentView();
    UMLSceneRectItem *rect;
    rect = new UMLSceneRectItem(p.x() - SIZE / 2,
                                p.y() - SIZE / 2,
                                SIZE, SIZE);
    rect->setCanvas(currentView->umlScene()->canvas());
    rect->setBrush( QBrush(Qt::blue) );
    rect->setPen( QPen(Qt::blue) );
    rect->setVisible(true);
    return rect;
}

/**
 * Returns true if the first widget's X is smaller than second's.
 * Used for sorting the UMLWidgetList.
 * @param widget1 The widget to compare.
 * @param widget2 The widget to compare with.
 */
bool hasSmallerX(const UMLWidget* widget1, const UMLWidget* widget2)
{
    return widget1->x() < widget2->x();
}

/**
 * Returns true if the first widget's Y is smaller than second's.
 * Used for sorting the UMLWidgetList.
 * @param widget1 The widget to compare.
 * @param widget2 The widget to compare with.
 */
bool hasSmallerY(const UMLWidget* widget1, const UMLWidget* widget2)
{
    return widget1->y() < widget2->y();
}

}  // namespace Widget_Utils

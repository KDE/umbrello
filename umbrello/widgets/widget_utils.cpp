/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_utils.h"

// qt includes
#include <QtGui/QBrush>
#include <QtGui/QPen>

// app includes
#include "debug_utils.h"
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"
#include "objectwidget.h"


namespace Widget_Utils {

UMLWidget* findWidget(Uml::IDType id,
                      const UMLWidgetList& widgets,
                      const MessageWidgetList* pMessages /* = NULL */)
{
    UMLWidgetListIt it( widgets );
    foreach ( UMLWidget* obj , widgets ) {
        if (obj->baseType() == WidgetBase::wt_Object) {
            if (static_cast<ObjectWidget *>(obj)->localID() == id)
                return obj;
        } else if (obj->id() == id) {
            return obj;
        }
    }

    if (pMessages == NULL)
        return NULL;

    foreach ( UMLWidget* obj , *pMessages ) {
        if( obj->id() == id )
            return obj;
    }
    return NULL;
}

UMLSceneRectangle *decoratePoint(const QPoint& p)
{
    const int SIZE = 4;
    UMLView *currentView = UMLApp::app()->currentView();
    UMLSceneRectangle *rect;
    rect = new UMLSceneRectangle(p.x() - SIZE / 2,
                                 p.y() - SIZE / 2,
                                 SIZE, SIZE, currentView->canvas());
    rect->setBrush( QBrush(Qt::blue) );
    rect->setPen( QPen(Qt::blue) );
    rect->setVisible(true);
    return rect;
}


}  // namespace Widget_Utils


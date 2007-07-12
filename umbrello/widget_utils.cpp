/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_utils.h"

// qt/kde includes
#include <qcanvas.h>
#include <qbrush.h>
#include <qpen.h>
#include <kiconloader.h>
#include <kdebug.h>

// app includes
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
    UMLWidget * obj = NULL;
    while ( (obj = it.current()) != NULL ) {
        ++it;
        if (obj->getBaseType() == Uml::wt_Object) {
            if (static_cast<ObjectWidget *>(obj)->getLocalID() == id)
                return obj;
        } else if (obj->getID() == id) {
            return obj;
        }
    }

    if (pMessages == NULL)
        return NULL;

    MessageWidgetListIt mit( *pMessages );
    while ( (obj = (UMLWidget*)mit.current()) != NULL ) {
        ++mit;
        if( obj -> getID() == id )
            return obj;
    }
    return NULL;
}

QIconSet iconSet(Uml::Diagram_Type dt) {
    QIconSet diagramIconSet;
    switch (dt) {
    case Uml::dt_UseCase:
        diagramIconSet = BarIconSet("umbrello_diagram_usecase");
        break;
    case Uml::dt_Collaboration:
        diagramIconSet = BarIconSet("umbrello_diagram_collaboration");
        break;
    case Uml::dt_Class:
        diagramIconSet = BarIconSet("umbrello_diagram_class");
        break;
    case Uml::dt_Sequence:
        diagramIconSet = BarIconSet("umbrello_diagram_sequence");
        break;
    case Uml::dt_State:
        diagramIconSet = BarIconSet("umbrello_diagram_state");
        break;
    case Uml::dt_Activity:
        diagramIconSet = BarIconSet("umbrello_diagram_activity");
        break;
    case Uml::dt_Component:
        diagramIconSet = BarIconSet("umbrello_diagram_component");
        break;
    case Uml::dt_Deployment:
        diagramIconSet = BarIconSet("umbrello_diagram_deployment");
        break;
    case Uml::dt_EntityRelationship:
        diagramIconSet = BarIconSet("umbrello_diagram_entityrelationship");
        break;
    default:
        kDebug() << "Widget_Utils::iconSet: unknown diagram type " << dt << endl;
        diagramIconSet = BarIconSet("unknown");
    }
    return diagramIconSet;
}

QCanvasRectangle *decoratePoint(const QPoint& p) {
    const int SIZE = 4;
    UMLView *currentView = UMLApp::app()->getCurrentView();
    QCanvasRectangle *rect;
    rect = new QCanvasRectangle(p.x() - SIZE / 2,
                                p.y() - SIZE / 2,
                                SIZE, SIZE, currentView->canvas());
    rect->setBrush( QBrush(Qt::blue) );
    rect->setPen( QPen(Qt::blue) );
    rect->setVisible(true);
    return rect;
}


}  // namespace Widget_Utils


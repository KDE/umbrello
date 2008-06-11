/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_utils.h"

// qt/kde includes
#include <q3canvas.h>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <kdebug.h>
// app includes
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"
#include "objectwidget.h"
#include "umlscene.h"


namespace Widget_Utils
{

    UMLWidget* findWidget(Uml::IDType id,
                          const UMLWidgetList& widgets,
                          const MessageWidgetList* pMessages /* = NULL */)
    {
        UMLWidgetListIt it( widgets );
        foreach ( UMLWidget* obj , widgets ) {
            if (obj->getBaseType() == Uml::wt_Object) {
                if (static_cast<ObjectWidget *>(obj)->getLocalID() == id)
                    return obj;
            } else if (obj->getID() == id) {
                return obj;
            }
        }

        if (pMessages == NULL)
            return NULL;

        foreach ( UMLWidget* obj , *pMessages ) {
            if( obj -> getID() == id )
                return obj;
        }
        return NULL;
    }

    QGraphicsRectItem *decoratePoint(const QPointF& p)
    {
        const int SIZE = 4;
        UMLView *currentView = UMLApp::app()->getCurrentView();
        QGraphicsRectItem *rect = new QGraphicsRectItem(0, 0, SIZE, SIZE);
        currentView->umlScene()->addItem(rect);
        rect->setPos(p.x() - SIZE / 2, p.y() - SIZE / 2);
        rect->setBrush( QBrush(Qt::blue) );
        rect->setPen( QPen(Qt::blue) );
        return rect;
    }

    void loadPainterInfoFromXMI(const QDomElement &qElement, QPen &pen,
                                QBrush &brush, QFont &font)
    {
        //TODO: Implement this
    }

    void savePainterInfoToXMI(QDomDocument &qDoc, QDomElement &qElement,
                              const QPen &pen, const QBrush &brush,
                              const QFont &font)
    {
        //TODO: Implement this
    }


}  // namespace Widget_Utils


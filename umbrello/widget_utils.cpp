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


    /**
     * Helper method.
     * @internal
     * @return The resize handle rectangle for given \a rect.
     */
    QRectF rectForResizeHandle(const Uml::ResizeHandle handle, const QRectF& rect)
    {
        const qreal d = RESIZE_HANDLE_SIZE;
        const qreal hd = 0.5 * (d - 1);
        QRectF handleRect(0, 0, d-1, d-1);
        QPointF delta(d-1, d-1);

        qreal x, y;


        switch(handle)
        {
        case Uml::rh_TopLeft:
            x = rect.left() - hd;
            y = rect.top() - hd;
            break;

        case Uml::rh_Top:
            x = rect.center().x();
            y = rect.top() - hd;
            break;

        case Uml::rh_TopRight:
            x = rect.right() + hd;
            y = rect.top() - hd;
            break;

        case Uml::rh_Right:
            x = rect.right() + hd;
            y = rect.center().y();
            break;

        case Uml::rh_BottomRight:
            x = rect.right() + hd;
            y = rect.bottom() + hd;
            break;

        case Uml::rh_Bottom:
            x = rect.center().x();
            y = rect.bottom() + hd;
            break;

        case Uml::rh_BottomLeft:
            x = rect.left() - hd;
            y = rect.bottom() + hd;
            break;

        case Uml::rh_Left:
            x = rect.left() - hd;
            y = rect.center().y();

        default:
            ; // NOP to avoid compiler warning

        }
        handleRect.moveCenter(QPointF(x, y));
        return handleRect;
    }

    void drawResizeHandles(QPainter *painter, const QRectF &rect)
    {
        // painter->save();
        for(unsigned i = Uml::rh_TopLeft; i <= unsigned(Uml::rh_Left); ++i) {
            QRectF handleRect = rectForResizeHandle((Uml::ResizeHandle)i, rect);
            painter->fillRect(handleRect, Qt::blue);
        }
    }

    Uml::ResizeHandle resizeHandleForPoint(const QPointF &point, const QRectF& rect)
    {
        for(unsigned i = Uml::rh_TopLeft; i <= unsigned(Uml::rh_Left); ++i) {
            QRectF handleRect = rectForResizeHandle((Uml::ResizeHandle)i, rect);
            if(handleRect.contains(point)) {
                return Uml::ResizeHandle(i);
            }
        }
        return Uml::rh_None;
    }

    QCursor cursorForResizeHandle(const Uml::ResizeHandle handle)
    {
        switch (handle)
        {
        case Uml::rh_TopLeft:
        case Uml::rh_BottomRight:
            return QCursor(Qt::SizeFDiagCursor);

        case Uml::rh_Top:
        case Uml::rh_Bottom:
            return QCursor(Qt::SizeVerCursor);

        case Uml::rh_TopRight:
        case Uml::rh_BottomLeft:
            return QCursor(Qt::SizeBDiagCursor);

        case Uml::rh_Right:
        case Uml::rh_Left:
            return QCursor(Qt::SizeHorCursor);

        default:
            return QCursor(Qt::ArrowCursor);
        }
    }

    void adjustRectForResizeHandles(QRectF &rect)
    {
        const qreal d = RESIZE_HANDLE_SIZE;
        rect.adjust(-d, -d, d, d);
    }

}  // namespace Widget_Utils

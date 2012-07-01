/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#ifndef WIDGETHANDLE_H
#define WIDGETHANDLE_H

#include <QGraphicsItem>

class UMLWidget;

class WidgetHandle : public QGraphicsItem
{
public:
    static const qreal HandleSize;

    enum ResizeHandle {
        rh_TopLeft = 0,
        rh_Top,
        rh_TopRight,
        rh_Right,
        rh_BottomRight,
        rh_Bottom,
        rh_BottomLeft,
        rh_Left,

        rh_None
    };

    WidgetHandle(UMLWidget *widget);

    QRectF boundingRect() const {
        return m_boundingRect;
    }

    QPainterPath shape() const {
        return m_shape;
    }

    void updateHandlePosition() {
        calcResizeHandles();
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    bool isActive() const;
    void calcResizeHandles();
    void handleCursorChange(QGraphicsSceneHoverEvent *event);

    UMLWidget *m_widget;
    QRectF m_boundingRect;
    QPainterPath m_shape;
    QRectF m_resizeHandles[8];
    ResizeHandle m_pressedHandle;
};

#endif //WIDGETHANDLE_H

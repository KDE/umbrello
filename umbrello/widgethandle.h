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

#include <QtGui/QGraphicsItem>

class NewUMLRectWidget;

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

    WidgetHandle(NewUMLRectWidget *widget);

    QRectF boundingRect() const {
        return m_boundingRect;
    }

    QPainterPath shape() const {
        return m_shape;
    }

    void updateHandlePosition() {
        calcResizeHandles();
    }

    /**
     * Reimplemented to draw the resize handles.
     *
     * @note The handles are drawn only if this WidgetHandle object is
     * active.
     *
     * @see WidgetHandle::isActive()
     */
    void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *w);

protected:

    /**
     * Reimplemented to handles resizing on dragging the resize handle, if any.
     *
     * @note This method handles resizing only if this handle is active
     *       and resizing is enabled on the associated NewUMLRectWidget
     *       object.
     *
     * @see WidgetHandle::isActive()
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

    /**
     * @copydoc WidgetHandle::mousePressEvent
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    /**
     * @copydoc WidgetHandle::mousePressEvent
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    /**
     * Reimplemented to handle the cursor change on hovering mouse over
     * resize handle.
     *
     * @see WidgetHandle::handleCursorChange
     */
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

    /**
     * @copydoc WidgetHandle::hoverEnterEvent
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

    /**
     * @copydoc WidgetHandle::hoverEnterEvent
     */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:

    /**
     * @return The active status of the WidgetHandle (used to determine
     *         whether to draw, react)
     *
     * @retval true If a widget is associated with this handle and that
     *              widget is selected
     *
     * @retval false Otherwise.
     */
    bool isActive() const;

    /**
     * Calculates the position and geometry of resize handle rects.  In
     * the process, the complete geometry of this WidgetHandle is
     * calculated.
     *
     * @note This method calculates only if a widget is associated with
     *       this handle. And this doesn't check to see if that widget is
     *       selected or not as that check is already done by event
     *       handlers and also this might be needed for bookkeeping
     */
    void calcResizeHandles();

    /**
     * Sets a proper cursor for this WidgetHandle item based on
     * QGraphicsSceneHoverEvent parameter.
     *
     * @note The cursor set is Qt::ArrowCursor if either this handle is
     *       not active or the associated widget is not resizable.
     */
    void handleCursorChange(QGraphicsSceneHoverEvent *event);

    NewUMLRectWidget *m_widget;
    QRectF m_boundingRect;
    QPainterPath m_shape;

    QRectF m_resizeHandles[8];
    ResizeHandle m_pressedHandle;
};

#endif //WIDGETHANDLE_H

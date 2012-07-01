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

#include "widgethandle.h"

#include "debug_utils.h"
#include "umlwidget.h"

#include <QCursor>
#include <QPainter>

/**
 * @class WidgetHandle
 *
 * @short This class provides the resize handles for a
 *        UMLWidget.
 *
 * @details This class handles the cursor change and checks for the
 *          sizeHint constraints of the widget class while resizing
 *          it.  The widget object should simply create/show this
 *          WidgetHandle object when it is selected and delete/hide
 *          this on deselection.
 *
 *          There might be situation where the associted
 *          UMLWidget is not resizable. In such cases, the
 *          resize handles are drawn on creating this object (to show
 *          selection) but aren't resized when the resize handle is
 *          dragged.
 *
 * @note The resize handles aren't drawn and don't react if this
 *       handle is not active.
 * @see  WidgetHandle::isActive()
 */


/**
 * @enum WidgetHandle::ResizeHandle
 *
 * An enumeration to represent the areas of resize handles.
 *
 * @note rh_None is also used as number of enum items and rh_TopLeft
 *       is used as the starting enum entry, so that we can easily use
 *       a for loop with these enum values.
 */

/**
 * @fn WidgetHandle::boundingRect
 *
 * Reimplemented to return the bounding rect (cached for performance
 * reasons.)
 */

/**
 * @fn WidgetHandle::shape
 *
 * Reimplemented to return the shape of this item (8 small resize
 * rects).  This is cached for performance reasons.
 */

/**
 * @var WidgetHandle::m_resizeHandles
 *
 * Represents the rectangles represiting the resize handles.
 */

/**
 * @var WidgetHandle::m_pressedHandle
 *
 * Represents the handle on which the mouse was left button pressed.
 */


/**
 * The size of the rectangle used to represent resize handles.
 */
const qreal WidgetHandle::HandleSize = 6;

/**
 * Constructs an object of WidgetHandle associated with \a widget.
 * The hover events are enabled and the array of resize handle rect's
 * are initialized to have their size equal to HandleSize.
 *
 * @param widget The UMLWidget object with which this
 *               WidgetHandle should be associated with. \a widget is
 *               also is the parent item for this WidgetHandle item.
 */
WidgetHandle::WidgetHandle(UMLWidget *widget)
  : QGraphicsItem(widget),
    m_widget(widget)
{
    for(int i = rh_TopLeft; i < rh_None; ++i) {
        m_resizeHandles[i].setRect(0, 0, HandleSize, HandleSize);
    }
    calcResizeHandles();
    setAcceptsHoverEvents(true);
}

/**
 * Reimplemented to draw the resize handles.
 *
 * @note The handles are drawn only if this WidgetHandle object is
 * active.
 *
 * @see WidgetHandle::isActive()
 */
void WidgetHandle::paint(QPainter *p, const QStyleOptionGraphicsItem* opt,
                         QWidget *wid)
{
    Q_UNUSED(opt);
    Q_UNUSED(wid);

    if(isActive()) {
        p->setBrush(Qt::blue);
        for(int i=rh_TopLeft; i < rh_None; ++i) {
            p->drawRect(m_resizeHandles[i]);
        }
    }
}

/**
 * Reimplemented to handles resizing on dragging the resize handle, if any.
 *
 * @note This method handles resizing only if this handle is active
 *       and resizing is enabled on the associated UMLWidget
 *       object.
 *
 * @see WidgetHandle::isActive()
 */
void WidgetHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(!isActive() || !m_widget->isResizable() || event->button() != Qt::LeftButton) {
        m_pressedHandle = rh_None;
        return;
    }

    m_pressedHandle = rh_None;

    for(int i=rh_TopLeft; i < rh_None; ++i) {
        if(m_resizeHandles[i].contains(event->pos())) {
            m_pressedHandle = (ResizeHandle)i;
            return;
        }
    }
}

/**
 * @copydoc WidgetHandle::mousePressEvent
 */
void WidgetHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // Should not happen
    if(m_pressedHandle == rh_None) {
        uWarning() << "Pressed handle is None in mouse move event";
        return;
    }

    // Do nothing is not active or resizing is disabled.
    if(!isActive() || !m_widget->isResizable()) {
        return;
    }

    const QPointF sp = event->scenePos();
    const QSizeF minSize = m_widget->minimumSize();
    const QSizeF maxSize = m_widget->maximumSize();

    qreal w, h, minw, maxw, minh, maxh;
    minw = minSize.width();
    maxw = maxSize.width();
    minh = minSize.height();
    maxh = maxSize.height();

    QRectF newRect(m_widget->scenePos(), m_widget->size());

    // Now ensure that the new geometry obeys the required sizeHint constraints
    switch(m_pressedHandle)
    {
    case rh_TopLeft:
        h = newRect.bottom() - sp.y();
        if(h >= minh && h <= maxh) {
            newRect.setTop(sp.y());
        }

        w = newRect.right() - sp.x();
        if(w >= minw && w <= maxw) {
            newRect.setLeft(sp.x());
        }
        break;

    case rh_Top:
        h = newRect.bottom() - sp.y();

        if(h >= minh && h <= maxh) {
            newRect.setTop(sp.y());
        }
        break;

    case rh_TopRight:
        h = newRect.bottom() - sp.y();
        if(h >= minh && h <= maxh) {
            newRect.setTop(sp.y());
        }

        w = sp.x() - newRect.left();
        if(w >= minw && w <= maxw) {
            newRect.setRight(sp.x());
        }
        break;

    case rh_Right:
        w = sp.x() - newRect.left();
        if(w >= minw && w <= maxw) {
            newRect.setRight(sp.x());
        }
        break;

    case rh_BottomRight:
        h = sp.y() - newRect.top();
        if(h >= minh && h <= maxh) {
            newRect.setBottom(sp.y());
        }

        w = sp.x() - newRect.left();
        if(w >= minw && w <= maxw) {
            newRect.setRight(sp.x());
        }
        break;

    case rh_Bottom:
        h = sp.y() - newRect.top();
        if(h >= minh && h <= maxh) {
            newRect.setBottom(sp.y());
        }
        break;

    case rh_BottomLeft:
        h = sp.y() - newRect.top();
        if(h >= minh && h <= maxh) {
            newRect.setBottom(sp.y());
        }

        w = newRect.right() - sp.x();
        if(w >= minw && w <= maxw) {
            newRect.setLeft(sp.x());
        }
        break;

    case rh_Left:
        w = newRect.right() - sp.x();
        if(w >= minw && w <= maxw) {
            newRect.setLeft(sp.x());
        }
        break;

    default:
        ; // NOP to prevent compiler warning
    }

    if (m_widget->parentItem()) {
        newRect.moveTopLeft(m_widget->parentItem()->mapFromScene(newRect.topLeft()));
    }
    // Apply the geometry change now to the associated widget.
    m_widget->setPos(newRect.topLeft());
    m_widget->setSize(newRect.size());

    // Calculate the position and hence geometry of resize handles rects.
    calcResizeHandles();
}

/**
 * @copydoc WidgetHandle::mousePressEvent
 */
void WidgetHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    // reset the pressed handle
    m_pressedHandle = rh_None;
}

/**
 * Reimplemented to handle the cursor change on hovering mouse over
 * resize handle.
 *
 * @see WidgetHandle::handleCursorChange
 */
void WidgetHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    handleCursorChange(event);
}

/**
 * @copydoc WidgetHandle::hoverEnterEvent
 */
void WidgetHandle::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    handleCursorChange(event);
}

/**
 * @copydoc WidgetHandle::hoverEnterEvent
 */
void WidgetHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    handleCursorChange(event);
}

/**
 * @return The active status of the WidgetHandle (used to determine
 *         whether to draw, react)
 *
 * @retval true If a widget is associated with this handle and that
 *              widget is selected
 *
 * @retval false Otherwise.
 */
bool WidgetHandle::isActive() const
{
    return m_widget != 0 && m_widget->isSelected();
}

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
void WidgetHandle::calcResizeHandles()
{
    // Cant calculate if no widget is associated with this.
    if(!m_widget) {
        return;
    }

    const qreal size = WidgetHandle::HandleSize;
    const qreal half = 0.5 * size;

    QRectF handleRect(0, 0, size, size);



    QRectF rect = mapFromItem(this, m_widget->rect()).boundingRect();

    m_resizeHandles[rh_TopLeft].
        moveTopLeft(QPointF(rect.left() - size, rect.top() - size));

    m_resizeHandles[rh_Top].
        moveTopLeft(QPointF(rect.center().x() - half, rect.top() - size));

    m_resizeHandles[rh_TopRight].
        moveTopLeft(QPointF(rect.right(), rect.top() - size));

    m_resizeHandles[rh_Right].
        moveTopLeft(QPointF(rect.right(), rect.center().y() - half));

    m_resizeHandles[rh_BottomRight].
        moveTopLeft(QPointF(rect.right(), rect.bottom()));

    m_resizeHandles[rh_Bottom].
        moveTopLeft(QPointF(rect.center().x() - half, rect.bottom()));

    m_resizeHandles[rh_BottomLeft].
        moveTopLeft(QPointF(rect.left() - size, rect.bottom()));

    m_resizeHandles[rh_Left].
        moveTopLeft(QPointF(rect.left() - size, rect.center().y() - half));

    // Destroy old object and readd the resize handles to newly created.
    m_shape = QPainterPath();
    for(int i=rh_TopLeft; i < rh_None; ++i) {
        m_shape.addRect (m_resizeHandles[i]);
    }

    prepareGeometryChange();
    // Slight adjustment to allow better painting control.
    const qreal adj = .5;
    // Use the painter path to get the bounding rect.
    m_boundingRect = m_shape.boundingRect().adjusted(-adj, -adj, adj, adj);
    update();
}

/**
 * Sets a proper cursor for this WidgetHandle item based on
 * UMLSceneHoverEvent parameter.
 *
 * @note The cursor set is Qt::ArrowCursor if either this handle is
 *       not active or the associated widget is not resizable.
 */
void WidgetHandle::handleCursorChange(QGraphicsSceneHoverEvent *event)
{
    if(!isActive() || !m_widget->isResizable()) {
        setCursor(QCursor(Qt::ArrowCursor));
        return;
    }
    ResizeHandle hoveredHandle = rh_None;

    for(int i=rh_TopLeft; i < rh_None; ++i) {
        if(m_resizeHandles[i].contains(event->pos())) {
            hoveredHandle = (ResizeHandle)i;
            break;
        }
    }

    Qt::CursorShape cursorShape;
    switch (hoveredHandle)
    {
    case rh_TopLeft:
    case rh_BottomRight:
        cursorShape = Qt::SizeFDiagCursor;
        break;

    case rh_Top:
    case rh_Bottom:
        cursorShape = Qt::SizeVerCursor;
        break;

    case rh_TopRight:
    case rh_BottomLeft:
        cursorShape = Qt::SizeBDiagCursor;
        break;

    case rh_Right:
    case rh_Left:
        cursorShape = Qt::SizeHorCursor;
        break;

    default:
        cursorShape = Qt::ArrowCursor;
    }

    setCursor(QCursor(cursorShape));
}

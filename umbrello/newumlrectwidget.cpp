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

#include "newumlrectwidget.h"
#include "listpopupmenu.h"
#include "widget_utils.h"
#include "umlscene.h"

#include <QtGui/QDialog>
#include <QtGui/QGraphicsSceneHoverEvent>

#include <kdebug.h>

const QSizeF NewUMLRectWidget::DefaultMinimumSize(50, 20);
const QSizeF NewUMLRectWidget::DefaultMaximumSize(1000, 1000);
const QSizeF NewUMLRectWidget::DefaultPreferredSize(70, 20);

NewUMLRectWidget::NewUMLRectWidget(UMLObject *object) :
    NewUMLWidget(object),
    m_size(20, 20),
    m_resizable(true),
    m_resizeHandle(Uml::rh_None)
{
}

NewUMLRectWidget::~NewUMLRectWidget()
{
}

QSizeF NewUMLRectWidget::sizeHint(Qt::SizeHint which)
{
    switch(which) {
    case Qt::MinimumSize:
        return DefaultMinimumSize;

    case Qt::MaximumSize:
        return DefaultMaximumSize;

    case Qt::PreferredSize:
    default:
        return DefaultPreferredSize;
    }
}

void NewUMLRectWidget::setSize(const QSizeF &size)
{
    QSizeF oldSize = m_size;

    m_size = size;
    adjustSizeForConstraints(m_size);

    QRectF boundRect = rect();
    /*
     * There is no need to adjust half the pen width if the widget is
     * resizable since the small resize hint rectangles themselves
     * ensure the geometry of widget adjusts even for half pen width.
     */
    if(isResizable()) {
        Widget_Utils::adjustRectForResizeHandles(boundRect);
    }
    else {
        // Adjust bounding rect with half the pen width(lineWidth).
        qreal hpw = 0.5 * lineWidth();
        boundRect.adjust(-hpw, -hpw, hpw, hpw);
    }
    setBoundingRect(boundRect);

    // Notify by calling sizeChanged method with old size
    sizeChanged(oldSize);
}

void NewUMLRectWidget::setInstanceName(const QString &name)
{
    m_instanceName = name;
    updateGeometry();
}

void NewUMLRectWidget::addAssociationWidget(AssociationWidget *assoc)
{
    if(!m_associationWidgetList.contains(assoc)) {
        m_associationWidgetList << assoc;
    }
}

void NewUMLRectWidget::removeAssociationWidget(AssociationWidget *assoc)
{
    m_associationWidgetList.removeAll(assoc);
}

void NewUMLRectWidget::showPropertiesDialog()
{
    QDialog stubDialog;
    stubDialog.setWindowTitle("Stub.. To be worked out still.");
    stubDialog.exec();
}

void NewUMLRectWidget::setupContextMenuActions(ListPopupMenu &menu)
{
    menu.addAction("Stub!!");
}

void NewUMLRectWidget::adjustConnectedAssociations()
{
    //TODO: Implement this once AssociationWidget's are implemented.
}

bool NewUMLRectWidget::loadFromXMI(QDomElement &qElement)
{
    if(!NewUMLWidget::loadFromXMI(qElement))
        return false;

    qreal h  = qElement.attribute("height", "-1").toDouble();
    qreal w = qElement.attribute("width", "-1").toDouble();
    if(h < 0) {
        h = 20;
    }
    if(w < 0) {
        w = 20;
    }
    m_size.setWidth(w);
    m_size.setHeight(h);
    updateGeometry(); // Force updation of the shape as well.

    QString instName = qElement.attribute("instancename", QString());
    setInstanceName(instName);

    return true;
}

void NewUMLRectWidget::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    NewUMLWidget::saveToXMI(qDoc, qElement);

    const QSizeF sz = size();
    qElement.setAttribute("width", sz.width());
    qElement.setAttribute("height", sz.height());

    if(!m_instanceName.isEmpty()) {
        qElement.setAttribute("instancename", m_instanceName);
    }
}

void NewUMLRectWidget::sizeChanged(const QSizeF& oldSize)
{
    Q_UNUSED(oldSize);

    QPainterPath newShape;
    newShape.addRect(boundingRect());
    setShape(newShape);
}

void NewUMLRectWidget::updateGeometry()
{
    // The idea here is to simply call setSize with current size which
    // will take care of the sizeHint constraints.

    // Subclasses might calculate their sizeHint constraints
    setSize(m_size);
}

void NewUMLRectWidget::setResizable(bool resizable)
{
    m_resizable = resizable;
    updateGeometry();
}

void NewUMLRectWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_resizeHandle != Uml::rh_None) {
        return;
    }
    NewUMLWidget::mousePressEvent(event);
    event->accept();
    // Don't handle resizing if the widget is non-resizable or there
    // are more selected widgets.
    if(!isResizable() || scene()->selectedItems().size() > 1) {
        return;
    }

    if(event->button() == Qt::LeftButton) {
        m_resizeHandle = Widget_Utils::resizeHandleForPoint(event->pos(), rect());
        if(m_resizeHandle != Uml::rh_None) {
            m_oldGeometry = rect(); // save the current geometry
        }
    }
}

void NewUMLRectWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *e)
{
    // Use the default widget move behavior if the widget is not being
    // resized.
    if(m_resizeHandle == Uml::rh_None) {
        NewUMLWidget::mouseMoveEvent(e);
        return;
    }

    e->accept();

    const QPointF sp = e->scenePos();
    const QSizeF minSize = sizeHint(Qt::MinimumSize);
    const QSizeF maxSize = sizeHint(Qt::MaximumSize);
    qreal w, h, minw, maxw, minh, maxh;
    minw = minSize.width();
    maxw = maxSize.width();
    minh = minSize.height();
    maxh = maxSize.height();

    QRectF newRect(scenePos(), size());

    // Now ensure that the new geometry obeys the required sizeHint constraints
    switch(m_resizeHandle)
    {
    case Uml::rh_TopLeft:
        h = newRect.bottom() - sp.y();
        if(h >= minh && h <= maxh) {
            newRect.setTop(sp.y());
        }

        w = newRect.right() - sp.x();
        if(w >= minw && w <= maxw) {
            newRect.setLeft(sp.x());
        }
        break;

    case Uml::rh_Top:
        h = newRect.bottom() - sp.y();

        if(h >= minh && h <= maxh) {
            newRect.setTop(sp.y());
        }
        break;

    case Uml::rh_TopRight:
        h = newRect.bottom() - sp.y();
        if(h >= minh && h <= maxh) {
            newRect.setTop(sp.y());
        }

        w = sp.x() - newRect.left();
        if(w >= minh && w <= maxw) {
            newRect.setRight(sp.x());
        }
        break;

    case Uml::rh_Right:
        w = sp.x() - newRect.left();
        if(w >= minh && w <= maxw) {
            newRect.setRight(sp.x());
        }
        break;

    case Uml::rh_BottomRight:
        h = sp.y() - newRect.top();
        if(h >= minh && h <= maxh) {
            newRect.setBottom(sp.y());
        }

        w = sp.x() - newRect.left();
        if(w >= minh && w <= maxw) {
            newRect.setRight(sp.x());
        }
        break;

    case Uml::rh_Bottom:
        h = sp.y() - newRect.top();
        if(h >= minh && h <= maxh) {
            newRect.setBottom(sp.y());
        }
        break;

    case Uml::rh_BottomLeft:
        h = sp.y() - newRect.top();
        if(h >= minh && h <= maxh) {
            newRect.setBottom(sp.y());
        }

        w = newRect.right() - sp.x();
        if(w >= minw && w <= maxw) {
            newRect.setLeft(sp.x());
        }
        break;

    case Uml::rh_Left:
        w = newRect.right() - sp.x();
        if(w >= minw && w <= maxw) {
            newRect.setLeft(sp.x());
        }
        break;

    default:
        ; // NOP to prevent compiler warning
    }

    setPos(newRect.topLeft());
    setSize(newRect.size());
}

void NewUMLRectWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(!isResizable() || m_resizeHandle == Uml::rh_None) {
        NewUMLWidget::mouseReleaseEvent(event);
    }
    event->accept();
    m_resizeHandle = Uml::rh_None;
}

void NewUMLRectWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}

void NewUMLRectWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    handleCursorChange(event);
}

void NewUMLRectWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    handleCursorChange(event);
}

void NewUMLRectWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(event->widget() ?
              event->widget()->cursor() :
              QCursor(Qt::ArrowCursor));
}

QVariant NewUMLRectWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == QGraphicsItem::ItemSelectedHasChanged) {
        bool selection = value.toBool();
        bool enableHover = selection && isResizable();
        // If multiple items are selected, resizing is not done.
        if(scene()) {
            enableHover = enableHover &&(scene()->selectedItems().size() == 1);
        }
        setAcceptHoverEvents(enableHover);
        if(!enableHover) {
            setCursor(QCursor(Qt::ArrowCursor));
        }
    }
    return NewUMLWidget::itemChange(change, value);
}

void NewUMLRectWidget::adjustSizeForConstraints(QSizeF &sz)
{
    const QSizeF minSize = sizeHint(Qt::MinimumSize);
    const QSizeF maxSize = sizeHint(Qt::MaximumSize);

    sz = minSize.expandedTo(sz);
    sz = maxSize.boundedTo(sz);
}

void NewUMLRectWidget::handleCursorChange(QGraphicsSceneHoverEvent *event)
{
    Uml::ResizeHandle handle = Widget_Utils::resizeHandleForPoint(event->pos(), rect());
    QCursor cursor = Widget_Utils::cursorForResizeHandle(handle);
    setCursor(cursor);
}

#include "newumlrectwidget.moc"

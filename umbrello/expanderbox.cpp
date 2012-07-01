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

#include "expanderbox.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

/// The size for all ExpanderBox
const qreal ExpanderBox::Size = 10;

/**
 * Constructs an ExpanderBox with the initial expanded state equal to
 * \a expanded and parent item being \a parent.
 */
ExpanderBox::ExpanderBox(bool expanded, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    m_expanded(expanded),
    m_brush(Qt::NoBrush)
{
    setPen(QPen(Qt::black, 1)); // sets boundingrect.
}

/**
 * Destructor.
 */
ExpanderBox::~ExpanderBox()
{
}

/**
 * Sets the current expanded state to \a exp and updates.
 */
void ExpanderBox::setExpanded(bool exp)
{
    if (m_expanded != exp) {
        m_expanded = exp;
        update();
    }
}

/**
 * Set's the pen used to draw box and updates the boundingRect as it
 * depends on pen width.
 */
void ExpanderBox::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        prepareGeometryChange();

        qreal adj = .5 * m_pen.widthF();
        m_boundingRect = rect();
        m_boundingRect.adjust(-adj, -adj, +adj, +adj);

        m_shape = QPainterPath();
        m_shape.addRect(rect());
    }
}

/**
 * Sets the brush used to fill the box and updates this box.
 */
void ExpanderBox::setBrush(const QBrush& brush)
{
    if (m_brush != brush) {
        m_brush = brush;
        update();
    }
}

/**
 * Reimplemented from QGraphicsItem::paint to draw the box and also
 * draw +/- based on current state.
 */
void ExpanderBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget*)
{
    const QRectF r = rect();
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawRoundedRect(r, 20, 20, Qt::RelativeSize);

    painter->drawLine(QLineF(r.left() + .2 * r.width(), r.center().y(),
                             r.right() - .2 * r.width(), r.center().y()));

    if (!m_expanded) {
        painter->drawLine(QLineF(r.center().x(), r.top() + .2 * r.height(),
                                 r.center().x(), r.bottom() - .2 * r.height()));
    }
}

/**
 * Reimplemented from QGraphicsItem::mousePressEvent to toggle current
 * state on left click and also emit the signal @ref expansionToggled
 */
void ExpanderBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_expanded = !m_expanded;
        update();
        emit expansionToggled(m_expanded);
        event->accept();
    }
    else {
        event->ignore();
    }
}

/**
 * Reimplemented from QGraphicsItem::mouseMoveEvent to ignore it.
 */
void ExpanderBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

/**
 * Reimplemented from QGraphicsItem::mouseReleaseEvent to ignore it.
 */
void ExpanderBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

#include "expanderbox.moc"

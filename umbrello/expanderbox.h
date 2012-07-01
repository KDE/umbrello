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

#ifndef EXPANDERBOX_H
#define EXPANDERBOX_H

#include <QBrush>
#include <QGraphicsObject>
#include <QObject>
#include <QPen>

/**
 * This is a simple item which displays a box with +/- on it and
 * toggles this on clicking the box.
 *
 * This can be used as GUI for the user to expand/deexpand
 * something. Currently this is used in ClassifierWidget.
 */
class ExpanderBox : public QGraphicsObject
{
    Q_OBJECT
public:
    ExpanderBox(bool expanded = false, QGraphicsItem *parent = 0);
    virtual ~ExpanderBox();

    /// @return The current state of expansion
    bool isExpanded() const {
        return m_expanded;
    }
    void setExpanded(bool expansion);

    /// @return The pen used to draw the box.
    QPen pen() const {
        return m_pen;
    }
    void setPen(const QPen& pen);

    /// @return The brush used to fill the box.
    QBrush brush() const {
        return m_brush;
    }
    void setBrush(const QBrush& brush);

    /// @return The geometry of the box (constant size)
    QRectF rect() const {
        return QRectF(0, 0, ExpanderBox::Size, ExpanderBox::Size);
    }

    /// Reimplemented from QGraphicsItem::boundingRect appropriately.
    virtual QRectF boundingRect() const {
        return m_boundingRect;
    }

    /// Reimplemented from QGraphicsItem::shape appropriately.
    virtual QPainterPath shape() const {
        return m_shape;
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *opt, QWidget*);

signals:
    void expansionToggled(bool currentState);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    static const qreal Size;

    bool m_expanded;
    QPen m_pen;
    QBrush m_brush;
    QRectF m_boundingRect;
    QPainterPath m_shape;
};

#endif //EXPANDERBOX_H

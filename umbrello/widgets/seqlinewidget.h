/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef SEQLINEWIDGET_H
#define SEQLINEWIDGET_H

#include <QtGui/QGraphicsItem>

class ObjectWidget;

/**
 * @short Widget class for graphical representation of sequence lines
 *
 * @author Paul Hensgen
 * @author Gopala Krishna
 *
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 *
 * @todo Should rename to SeqLineItem to differentiate between actual
 *       uml widgets and other items.
 */
class SeqLineWidget : public QGraphicsItem
{
public:
    SeqLineWidget(ObjectWidget * pObject);
    virtual ~SeqLineWidget();

    /// @return The color of the lines drawn
    QColor lineColor() const {
        return m_lineColor;
    }
    void setLineColor(const QColor & col);

    /// @return Width of the line drawn
    int lineWidth() const {
        return m_lineWidth;
    }
    void setLineWidth(int w);

    void updateDestructionBoxVisibility();
    bool onDestructionBox(const QPointF& localPos);

    /// @return Length of the sequential line.
    qreal length() const {
        return m_length;
    }
    void setLength(qreal length);
    void setEndOfLine(qreal yPosition);

    /// @return  Pointer to the associated ObjectWidget.
    ObjectWidget * objectWidget() const{
        return m_objectWidget;
    }

    virtual QRectF boundingRect() const {
        return m_boundingRect;
    }

    virtual QPainterPath shape() const {
        return m_shape;
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*);

private:
    static const qreal DestructionBoxSize;

    void updateGeometry();

    ObjectWidget * m_objectWidget;
    qreal m_length;

    QColor m_lineColor;
    int m_lineWidth;

    QLineF m_sequentialLine;
    QLineF m_destructionBoxLines[2];

    QRectF m_boundingRect;
    QPainterPath m_shape;
};

#endif

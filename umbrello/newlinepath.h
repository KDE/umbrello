/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef LINEPATH_H
#define LINEPATH_H

#include "umlnamespace.h"

#include <QtGui/QGraphicsItem>
#include <QtGui/QPen>

// Qt forward declarations
class QDomDocument;
class QDomElement;

namespace New
{
    class LinePath : public QGraphicsItem
    {
    public:
        explicit LinePath(QGraphicsItem *parent = 0);
        virtual ~LinePath();

        QPointF point(int index) const;
        void setPoint(int index, const QPointF& point);

        void insertPoint(int index, const QPointF& point);
        void removePoint(int index);
        void removeNonEndPoint(int index);
        void clear();

        void removeDuplicatePoints();
        void optimizeLinePoints();

        int closestPointIndex(const QPointF& point, qreal delta = LinePath::Delta) const;
        int segmentIndex(const QPointF& point, qreal delta = LinePath::Delta) const;

        bool isEndPoint(const QPointF& point) const;
        bool isEndPointIndex(int index) const;

        void setEndPoints(const QPointF &start, const QPointF &end);

        int count() const;
        int insertableLinePathIndex(const QPointF &position) const;

        bool loadFromXMI(QDomElement &qElement);
        void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

        QPen pen() const;
        void setPen(const QPen& pen);

        QColor lineColor() const;
        void setLineColor(const QColor &color);

        uint lineWidth() const;
        void setLineWidth(uint width);

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *opt, QWidget *);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        QVariant itemChange(GraphicsItemChange change, const QVariant& value);

    private:
        void calculateBoundingRect();

        /// These points represents the linepath.
        QVector<QPointF> m_points;
        /// The pen used to draw lines
        QPen m_pen;

        /// Index of active point which can be dragged to modify linepath.
        int m_activePointIndex;
        /**
         * Index of active segment index.
         * @note m_activePointIndex and m_activePointIndex can't be
         *       active at same time!
         */
        int m_activeSegmentIndex;

        /// The bounding rectangle of this linepath
        QRectF m_boundingRect;
        /// The shape of this linepath.
        QPainterPath m_shape;

        /// State variable to keep track of first move of segment.
        bool m_hasSegmentMoved;

        /// The default delta for fuzzy recognition of points closer to point.
        static const qreal Delta;
        /// The radius of circles drawn to show "selection".
        static const qreal SelectedPointDiameter;
    };

}
#endif

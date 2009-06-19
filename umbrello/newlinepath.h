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

#include <QtGui/QBrush>
#include <QtGui/QGraphicsItem>
#include <QtGui/QPen>

// Forward declarations
class QDomDocument;
class QDomElement;
class UMLWidget;
class RegionPair;

namespace New
{
    // More forward declaration (New namespace)
    class AssociationWidget;

    typedef QPair<QPointF, QPointF> PointPair;

    class HeadSymbol : public QGraphicsItem
    {
    public:
        /**
         * This enumeration lists all the symbols that can be used as
         * Head on linepath.
         */
        enum Symbol {
            None = -1,
            Arrow,
            Diamond,
            // Subset,
            Circle,
            Count
        };

        HeadSymbol(Symbol symbol, QGraphicsItem *parent = 0);
        virtual ~HeadSymbol();

        Symbol symbol() const;
        void setSymbol(Symbol symbol);

        virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;

        void alignTo(const QLineF& line);
        QLineF axisLine() const;
        PointPair symbolEndPoints() const;

        QPen pen() const;
        void setPen(const QPen &pen);

        QBrush brush() const;
        void setBrush(const QBrush& brush);

    private:
        /// Pen used to draw HeadSymbol
        QPen m_pen;
        /// Brush used to fill HeadSymbol
        QBrush m_brush;
        /// The current symbol being represented by this item.
        Symbol m_symbol;

        /// A structure to hold a table of values for all symbols.
        struct SymbolProperty {
            QRectF boundRect;
            QPainterPath shape;
            QLineF axisLine;
            PointPair endPoints;
        };

        /// A hack to prevent crash due to prepareGeometryChange call in constructor.
        bool m_firstTime;

        /// A table which stores all symbol properties.
        static SymbolProperty symbolTable[HeadSymbol::Count];
        friend void setupSymbolTable();
    };


    class AssociationLine
    {
    public:
        AssociationLine(New::AssociationWidget *assoc);
        ~AssociationLine();

        QPointF point(int index) const;
        void setPoint(int index, const QPointF& point);
        QPointF startPoint() const;
        QPointF endPoint() const;

        void insertPoint(int index, const QPointF& point);
        void removePoint(int index);

        int count() const;
        void clear();

        void optimizeLinePoints();

        int closestPointIndex(const QPointF& point, qreal delta = AssociationLine::Delta) const;
        int segmentIndex(const QPointF& point, qreal delta = AssociationLine::Delta) const;

        bool isEndPointIndex(int index) const;
        bool isEndSegmentIndex(int index) const;

        void setEndPoints(const QPointF &start, const QPointF &end);

        void setStartHeadSymbol(HeadSymbol::Symbol symbol);
        void setEndHeadSymbol(HeadSymbol::Symbol symbol);
        void alignHeadSymbols();

        bool loadFromXMI(QDomElement &qElement);
        void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

        QPen pen() const;
        void updatePenSettings();

        QRectF boundingRect() const;
        QPainterPath shape() const;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *opt);

        // Convenience functions which takes care of various
        // states. The user of this class, just has to call these
        // correspoding methods from its events.

        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        void calculateBoundingRect();

        RegionPair determineRegions();
        void calculateEndPoints();
        void calculateInitialEndPoints();


    private:
        /// These points represents the linepath.
        QVector<QPointF> m_points;

        /// Index of active point which can be dragged to modify linepath.
        int m_activePointIndex;
        /**
         * Index of active segment index.
         * @note m_activePointIndex and m_activePointIndex can't be
         *       active at same time!
         */
        int m_activeSegmentIndex;

        /// The association widget for which this line represents.
        AssociationWidget *m_associationWidget;

        /// The symbol drawn at the end of "first" line segment.
        HeadSymbol *m_startHeadSymbol;
        /// The symbol drawn at the end of "last" line segment.
        HeadSymbol *m_endHeadSymbol;

        /// The bounding rectangle of this linepath
        QRectF m_boundingRect;
        /// The shape of this linepath.
        QPainterPath m_shape;
        QGraphicsLineItem *tracker;

        /// The default delta for fuzzy recognition of points closer to point.
        static const qreal Delta;
        /// The radius of circles drawn to show "selection".
        static const qreal SelectedPointDiameter;
        /// Minimum height for self association's loop.
        static const qreal SelfAssociationMinimumHeight;
    };

}
#endif

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCIATIONLINE_H
#define ASSOCIATIONLINE_H

#include "umlscene.h"

#include <QBrush>
#include <QGraphicsObject>
#include <QPen>

// forward declarations
class AssociationWidget;
class QDomDocument;
class QDomElement;
class UMLWidget;
class RegionPair;

typedef QPair<QPointF, QPointF> SymbolEndPoints;

/**
 * This class provides with various symbols that can be embedded in
 * AssociationLine.  It also provides with convenience methods to align
 * the symbol to AssociationLine.
 */
class Symbol : public QGraphicsItem
{
    public:
        /**
         * This enumeration lists all the symbols that can be used as
         * embedded on AssociationLine.
         */
        enum SymbolType {
            None = -1,
            OpenArrow,
            ClosedArrow,
            CrowFeet,
            Diamond,
            Subset,
            Circle,
            Count
        };

        Symbol(SymbolType symbolType, QGraphicsItem *parent = 0);
        virtual ~Symbol();

        SymbolType symbolType() const;
        void setSymbolType(SymbolType symbolType);

        virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;

        void alignTo(const QLineF& line);
        QLineF axisLine() const;
        SymbolEndPoints symbolEndPoints() const;

        QPen pen() const;
        void setPen(const QPen &pen);

        QBrush brush() const;
        void setBrush(const QBrush& brush);

    private:
        /// Pen used to draw Symbol
        QPen m_pen;
        /// Brush used to fill Symbol
        QBrush m_brush;
        /// The current symbol being represented by this item.
        SymbolType m_symbolType;

        /// A structure to hold a table of values for all symbols.
        struct SymbolProperty {
            QRectF boundRect;
            QPainterPath shape;
            QLineF axisLine;
            SymbolEndPoints endPoints;
        };

        /// A hack to prevent crash due to prepareGeometryChange call in constructor.
        bool m_firstTime;

        /// A table which stores all symbol properties.
        static SymbolProperty symbolTable[Symbol::Count];
        static void setupSymbolTable();
};

/**
 * A convenience class that encapsulates geometry management, handles
 * mouse and hover events, embeds and aligns symbols and finally draws the
 * lines and points.
 *
 * This class is infact a draw and event handling proxy for
 * AssociationWidget.
 *
 * @author Gopala Krishna
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssociationLine : public QGraphicsObject
{
    public:
        AssociationLine(AssociationWidget *assoc);
        virtual ~AssociationLine();

        UMLScenePoint point(int index) const;
        void setPoint(int index, const UMLScenePoint& point);
        UMLScenePoint startPoint() const;
        UMLScenePoint endPoint() const;

        void insertPoint(int index, const UMLScenePoint& point);
        void removePoint(int index);

        int count() const;
        void clear();

        void optimizeLinePoints();

        int closestPointIndex(const UMLScenePoint& point, qreal delta = AssociationLine::Delta) const;
        int segmentIndex(const QPointF& point, qreal delta = AssociationLine::Delta) const;

        bool isEndPointIndex(int index) const;
        bool isEndSegmentIndex(int index) const;

        void setEndPoints(const UMLScenePoint &start, const UMLScenePoint &end);

        bool loadFromXMI(QDomElement &qElement);
        void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

        QPen pen() const;
        void updatePenSettings();

        virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

        QRectF boundingRect() const;
        QPainterPath shape() const;

        // Convenience functions which takes care of various
        // states. The user of this class, just has to call these
        // correspoding methods from its events.

        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

        void hoverEnterEvent(UMLSceneHoverEvent *event);
        void hoverMoveEvent(UMLSceneHoverEvent *event);
        void hoverLeaveEvent(UMLSceneHoverEvent *event);

        void calculateBoundingRect();

        RegionPair determineRegions();
        void calculateEndPoints();
        void calculateInitialEndPoints();
        void calculateAssociationClassLine();

        bool onAssociationClassLine(const QPointF& pos) const;

        void reconstructSymbols();

    private:
        void setStartSymbol(Symbol::SymbolType symbolType);
        void setEndSymbol(Symbol::SymbolType symbolType);

        void createSubsetSymbol();
        void removeSubsetSymbol();

        void createCollaborationLine();
        void removeCollaborationLine();

        void alignSymbols();

        /// These points represents the association line.
        QVector<QPointF> m_points;

        /// Index of active point which can be dragged to modify association line.
        int m_activePointIndex;
        /**
         * Index of active segment index.
         * @note m_activePointIndex and m_activeSegmentIndex can't be
         *       active at same time!
         */
        int m_activeSegmentIndex;

        /// The association widget for which this line represents.
        AssociationWidget *m_associationWidget;

        /// The symbol drawn at the end of "first" line segment.
        Symbol *m_startSymbol;
        /// The symbol drawn at the end of "last" line segment.
        Symbol *m_endSymbol;
        /// The subset symbol.
        Symbol *m_subsetSymbol;

        /// The parallel arrow line drawn in case of collaboration message.
        QGraphicsLineItem *m_collaborationLineItem;
        /// The arrow head drawn at end of m_collaborationLineItem
        Symbol *m_collaborationLineHead;

        /// The bounding rectangle of this AssociationLine
        QRectF m_boundingRect;
        /// The shape of this AssociationLine.
        QPainterPath m_shape;
        /// The line used to represent Association class.
        QGraphicsLineItem *m_associationClassLine;
        QGraphicsLineItem *m_tracker;

        /// The default delta for fuzzy recognition of points closer to point.
        static const qreal Delta;
        /// The radius of circles drawn to show "selection".
        static const qreal SelectedPointDiameter;
        /// Minimum height for self association's loop.
        static const qreal SelfAssociationMinimumHeight;
};

#endif

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ASSOCIATIONLINE_H
#define ASSOCIATIONLINE_H

#include "basictypes.h"

#include <QGraphicsObject>
#include <QList>
#include <QPen>
#include <QPoint>

// forward declarations
class AssociationWidget;
class QDomDocument;
class QDomElement;
class QPainter;

/**
 * This class provides with various symbols that can be embedded in
 * AssociationLine.  It also provides with convenience methods to align
 * the symbol to AssociationLine.
 */
class Symbol : public QGraphicsItem
{
    public:
        typedef QPair<QPointF, QPointF> SymbolEndPoints;

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

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;

        void alignTo(const QLineF& line);
        SymbolEndPoints symbolEndPoints() const;

        QPen pen() const;
        void setPen(const QPen &pen);

        QBrush brush() const;
        void setBrush(const QBrush& brush);

    private:
        QPen       m_pen;         ///< pen used to draw Symbol
        QBrush     m_brush;       ///< brush used to fill Symbol
        SymbolType m_symbolType;  ///< current symbol being represented by this item

        /// A structure to hold a table of values for all symbols.
        struct SymbolProperty {
            QRectF boundRect;
            QPainterPath shape;
            QLineF axisLine;
            SymbolEndPoints endPoints;
        };

        static SymbolProperty symbolTable[Symbol::Count];  ///< a table which stores all symbol properties
        static void setupSymbolTable();
};

/**
 * A convenience class that encapsulates geometry management, handles
 * mouse and hover events, embeds and aligns symbols and finally draws the
 * lines and points.
 * Context menu events are hadled in AssociationWidget.
 *
 * This class is infact a draw and event handling proxy for
 * AssociationWidget.
 *
 * @note m_activePointIndex and m_activeSegmentIndex can't be
 *       active at same time!
 *
 * @author Gopala Krishna
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssociationLine : public QGraphicsObject
{
    Q_OBJECT
    Q_ENUMS(LayoutType)
public:
    enum LayoutType {
        Direct = 1,
        Orthogonal,
        Polyline,
        Spline
    };

    static QString toString(LayoutType layout);
    static LayoutType fromString(const QString& layout);

    explicit AssociationLine(AssociationWidget *association);
    virtual ~AssociationLine();

    QPointF point(int index) const;
    bool setPoint(int index, const QPointF& point);
    QPointF startPoint() const;
    QPointF endPoint() const;

    void insertPoint(int index, const QPointF& point);
    void removePoint(int index);

    int count() const;
    void cleanup();

    void optimizeLinePoints();

    int closestPointIndex(const QPointF& point, qreal delta = Delta) const;
    int closestSegmentIndex(const QPointF& point, qreal delta = Delta) const;

    bool isEndPointIndex(int index) const;
    bool isEndSegmentIndex(int index) const;

    bool setEndPoints(const QPointF &start, const QPointF &end);

    void dumpPoints();

    bool loadFromXMI(QDomElement &qElement);
    void saveToXMI(QDomDocument &qDoc, QDomElement &qElement);

    QBrush brush() const;
    QPen pen() const;

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    QPainterPath path() const;

    QRectF boundingRect() const;
    QPainterPath shape() const;

    LayoutType layout() const;
    void setLayout(LayoutType layout);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void calculateInitialEndPoints();
    void calculateAssociationClassLine();

    bool onAssociationClassLine(const QPointF& pos) const;

    void reconstructSymbols();

    enum Region {  ///< Enum to tell whether the line docks top/bottom or left/right.
        TopBottom, LeftRight
    };

    void setDockRegion(Region region);

private:
    void setStartSymbol(Symbol::SymbolType symbolType);
    void setEndSymbol(Symbol::SymbolType symbolType);

    void createSubsetSymbol();
    void removeSubsetSymbol();

    void createCollaborationLine();
    void removeCollaborationLine();

    void alignSymbols();

    AssociationWidget *m_associationWidget;      ///< association widget for which this line represents
    QVector<QPointF>   m_points;                 ///< points representing the association line
    int                m_activePointIndex;       ///< index of active point which can be dragged to modify association line
    int                m_activeSegmentIndex;     ///< index of active segment
    Symbol            *m_startSymbol;            ///< symbol drawn at the end of "first" line segment
    Symbol            *m_endSymbol;              ///< symbol drawn at the end of "last" line segment
    Symbol            *m_subsetSymbol;           ///< subset symbol
    QGraphicsLineItem *m_collaborationLineItem;  ///< parallel arrow line drawn in case of collaboration message
    Symbol            *m_collaborationLineHead;  ///< arrow head drawn at end of m_collaborationLineItem
    QGraphicsLineItem *m_associationClassLine;   ///< line used to represent Association class
    LayoutType         m_layout;

    static QPainterPath createCubicBezierCurve(QVector<QPointF> points);
    static QPainterPath createOrthogonalPath(QVector<QPointF> points);

    static const qreal Delta;  ///< default delta for fuzzy recognition of points closer to point
    static const qreal SelectedPointDiameter;         ///< radius of circles drawn to show "selection"
    static const qreal SelfAssociationMinimumHeight;  ///< minimum height for self association's loop
};

#endif

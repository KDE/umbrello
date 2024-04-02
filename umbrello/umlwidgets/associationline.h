/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
class QXmlStreamWriter;

/**
 * This class provides various symbols that can be embedded in AssociationLine.
 * It also provides convenience methods to align the symbol to AssociationLine.
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

        explicit Symbol(SymbolType symbolType, QGraphicsItem *parent = nullptr);
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

        static SymbolProperty *symbolTable;  ///< a table which stores all symbol properties
        static void setupSymbolTable();
};

/**
 * A convenience class that encapsulates geometry management, handles
 * mouse and hover events, embeds and aligns symbols and finally draws the
 * lines and points.
 * Context menu events are handled in AssociationWidget.
 *
 * This class is infact a draw and event handling proxy for
 * AssociationWidget.
 *
 * @note m_activePointIndex and m_activeSegmentIndex can't be
 *       active at same time!
 *
 * @author Gopala Krishna
 * @author Andi Fischer
 * Bugs and comments to uml-devel@lists.sf.net or https://bugs.kde.org
 */
class AssociationLine : public QGraphicsObject
{
    Q_OBJECT
public:
    static QString toString(Uml::LayoutType::Enum layout);
    static Uml::LayoutType::Enum fromString(const QString& layout);

    explicit AssociationLine(AssociationWidget *association);
    virtual ~AssociationLine();

    QPointF point(int index) const;
    bool setPoint(int index, const QPointF& point);
    QPointF startPoint() const;
    QPointF endPoint() const;

    void addPoint(const QPointF& point);
    void insertPoint(int index, const QPointF& point);
    void removePoint(int index);

    int count() const;
    void cleanup();

    void optimizeLinePoints();

    int closestPointIndex(const QPointF& point, qreal delta = Delta) const;
    int closestSegmentIndex(const QPointF& point, qreal delta = Delta) const;

    bool isEndPointIndex(int index) const;
    bool isEndSegmentIndex(int index) const;
    bool isAutoLayouted() const;
    
    bool enableAutoLayout();

    bool setEndPoints(const QPointF &start, const QPointF &end);

    void dumpPoints();

    bool loadFromXMI(QDomElement &qElement);
    void saveToXMI(QXmlStreamWriter& writer);

    QBrush brush() const;
    QPen pen() const;
    void setPen(const QPen &pen);
    void updatePenStyle();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    QPainterPath path() const;

    QRectF boundingRect() const;
    QPainterPath shape() const;

    Uml::LayoutType::Enum  layout() const;
    void setLayout(Uml::LayoutType::Enum layout);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void calculateInitialEndPoints();

    void reconstructSymbols();

private:
    void setStartSymbol(Symbol::SymbolType symbolType);
    void setEndSymbol(Symbol::SymbolType symbolType);

    void createSubsetSymbol();
    void removeSubsetSymbol();

    void createCollaborationLine();
    void removeCollaborationLine();

    void alignSymbols();

    void createSplinePoints();

    AssociationWidget *m_associationWidget;      ///< association widget which this line represents
    QVector<QPointF>   m_points;                 ///< points representing the association line
    int                m_activePointIndex;       ///< index of active point which can be dragged to modify association line
    int                m_activeSegmentIndex;     ///< index of active segment
    Symbol            *m_startSymbol;            ///< symbol drawn at the end of "first" line segment
    Symbol            *m_endSymbol;              ///< symbol drawn at the end of "last" line segment
    Symbol            *m_subsetSymbol;           ///< subset symbol
    QGraphicsLineItem *m_collaborationLineItem;  ///< parallel arrow line drawn in case of collaboration message
    Symbol            *m_collaborationLineHead;  ///< arrow head drawn at end of m_collaborationLineItem
    Uml::LayoutType::Enum m_layout;
    QPen               m_pen;                    ///< pen used to draw an association line
    bool               m_autoLayoutSpline;
    static QPainterPath createBezierCurve(QVector<QPointF> points);
    static QPainterPath createOrthogonalPath(QVector<QPointF> points);

    qreal              m_c1dx{0.0}, m_c1dy{0.0}, m_c2dx{0.0}, m_c2dy{0.0};
    static const qreal Delta;  ///< default delta for fuzzy recognition of points closer to point
    static const qreal SelectedPointDiameter;         ///< radius of circles drawn to show "selection"
    static const qreal SelfAssociationMinimumHeight;  ///< minimum height for self association's loop
};

#endif

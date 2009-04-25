/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef LINEPATH_H
#define LINEPATH_H

#include "umlnamespace.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtGui/QGraphicsItem>

/* how many pixels a user could click around a point */
#define POINT_DELTA 5

// forward declarations
class AssociationWidget;
class QDomDocument;
class QDomElement;
class QPainter;

// typedefs
// [PORT] Ensure correctness.
typedef QList<QGraphicsLineItem*> LineList;
typedef QList<QGraphicsRectItem*> RectList;

/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class LinePath : public QObject
{
    Q_OBJECT
public:
    LinePath();
    ~LinePath();

    bool operator==( const LinePath & rhs );

    LinePath & operator=( const LinePath & rhs );

    enum Region {  ///< Enum to tell whether the line docks top/bottom or left/right.
        TopBottom, LeftRight
    };

    void setDockRegion( Region region );

    bool hasPoints () const;
    void dumpPoints ();

    bool setPoint( int pointIndex, const QPointF &point );
    QPointF getPoint( int pointIndex ) const;

    bool isPoint( int pointIndex, const QPointF &point, qreal delta = 0 );

    bool insertPoint( int pointIndex, const QPointF &point );
    bool removePoint( int pointIndex, const QPointF &point, qreal delta = 0 );

    bool setStartEndPoints( const QPointF &start, const QPointF &end );

    int count() const;

    int onLinePath( const QPointF &position );

    void setAssocType( Uml::Association_Type type );

    void update();

    void setAssociation(AssociationWidget * association);

    /**
     * Returns the Association this class is linked to.
     */
    AssociationWidget * getAssociation() {
        return m_pAssociation;
    }

    void setSelected( bool select );

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );
    bool loadFromXMI( QDomElement & qElement );

    void activate();

    void cleanup();

    QPen getPen();

    void setLineColor( const QColor &color );
    void setLineWidth( uint width );

protected:

    /**
     * Draw a (hollow) circle.
     * We can't use QCanvasEllipse directly for this because it doesn't
     * use the pen, i.e. QCanvasEllipse only draws filled ellipses.
     */
    class Circle : public QGraphicsEllipseItem
    {
    public:
        explicit Circle(QGraphicsScene * scene, qreal radius = 0);
        void setRadius(qreal radius);
        qreal getRadius() const;
        void setX(qreal x);
        void setY(qreal y);

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget* );
    };

    /**
     * Draw the subset Symbol
     */
    class SubsetSymbol : public QGraphicsEllipseItem
    {
    public:
        explicit SubsetSymbol(QGraphicsScene* scene);

        /**
         * Sets the Inclination of the Subset Symbol w.r.t horizontal x axis
         * @param angle The inclination angle
         */
        void setInclination(qreal angle) {
            // [PORT]
            QGraphicsEllipseItem::rotate(angle);
            // inclination = angle;
        }

        // [PORT]
#if 0

        void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);
#endif

    };

    QGraphicsScene*  getScene();

    Uml::Association_Type getAssocType() const;

    QColor getLineColor();
    uint getLineWidth();

    void moveSelected( int pointIndex );

    void setupSelected();

    void calculateHead();

    void createHeadLines();
    void createSubsetSymbol();

    void growList(LineList &list, int by);

    void updateHead();
    void updateSubsetSymbol();

    void setupParallelLine();
    void calculateParallelLine();
    void updateParallelLine();

    /********Attributes*************/

    AssociationWidget* m_pAssociation;  ///< The association we are representing.
    LineList           m_LineList;      ///< Contains all the lines of the association.
    RectList           m_RectList;      ///< Selected boxes list.
    LineList           m_HeadList;      ///< Head lines.
    LineList           m_ParallelList;  ///< The parallel line.
    bool               m_bSelected;     ///< Selected status.
    // [PORT] Cant use QPolygonF currently as it has different interface methods.
    QPolygon           m_PointArray;    ///< Contains calculated points used to draw the line head.

    /**
     *   Contains calculated points used to draw the line head.
     */
    QPointF m_ArrowPointA, m_ArrowPointB, m_MidPoint, m_EgdePoint;

    QGraphicsPolygonItem* m_pClearPoly;  ///< A polygon object to blank out any lines we don't want to see.
    Circle*       m_pCircle;             ///< The transparent circle required by containment associations.
    SubsetSymbol* m_pSubsetSymbol;       ///< The subset notation required by Child to Category associations.
    // [PORT] Cant use QPolygonF here due to differences in interface.
    QPolygon      m_ParallelLines;       ///< Contains the calculated points for the parallel line on a collaboration message to use.
    Region        m_DockRegion;          ///< Region where the line docks.
    bool m_bHeadCreated;
    bool m_bSubsetSymbolCreated;
    bool m_bParallelLineCreated;

public slots:
    void slotLineColorChanged( Uml::IDType viewID );
    void slotLineWidthChanged( Uml::IDType viewID );
};

#endif

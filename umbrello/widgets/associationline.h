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

#include "basictypes.h"
#include "umlscene.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QPoint>
#include <umlviewcanvas.h>

/* how many pixels a user could click around a point */
#define POINT_DELTA 5

// forward declarations
class AssociationWidget;
class QDomDocument;
class QDomElement;
class QPainter;


/**
 * @author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AssociationLine : public QObject
{
    Q_OBJECT
public:
    // typedefs
    typedef QList<UMLSceneLine*> LineList;
    typedef QList<UMLSceneRectangle*> RectList;

    AssociationLine();
    ~AssociationLine();

    bool operator==( const AssociationLine & rhs );

    AssociationLine & operator=( const AssociationLine & rhs );

    enum Region {  ///< Enum to tell whether the line docks top/bottom or left/right.
        TopBottom, LeftRight
    };

    void setDockRegion( Region region );

    bool hasPoints () const;
    void dumpPoints ();

    bool setPoint( int pointIndex, const QPoint &point );
    QPoint getPoint( int pointIndex ) const;

    bool isPoint( int pointIndex, const QPoint &point, unsigned short delta = 0 );

    bool insertPoint( int pointIndex, const QPoint &point );
    bool removePoint( int pointIndex, const QPoint &point = QPoint(), unsigned short delta = 0 );

    bool setStartEndPoints( const QPoint &start, const QPoint &end );

    int count() const;

    int onLinePath( const QPoint &position );

    void setAssocType( Uml::AssociationType type );

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
    class Circle : public UMLSceneEllipse
    {
    public:
        explicit Circle(UMLViewCanvas * canvas, int radius = 0);
        void setRadius(int radius);
        int getRadius() const;
        void setX(int x);
        void setY(int y);

        void drawShape(QPainter& p);
    };

    /**
     * Draw the subset Symbol
     */
    class SubsetSymbol : public UMLSceneEllipse
    {
    public:
        explicit SubsetSymbol(UMLViewCanvas* canvas);

        /**
         * Sets the Inclination of the Subset Symbol w.r.t horizontal x axis
         * @param angle The inclination angle
         */
        void setInclination(int angle) {
            inclination = angle;
        }
    protected:
        void drawShape(QPainter& p);

    private:
        int inclination;
    };

    UMLViewCanvas * getScene();

    Uml::AssociationType getAssocType() const;

    QColor lineColor();
    uint lineWidth();

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
    QPolygon           m_PointArray;    ///< Contains calculated points used to draw the line head.

    /**
     *   Contains calculated points used to draw the line head.
     */
    QPoint m_ArrowPointA, m_ArrowPointB, m_MidPoint, m_EgdePoint;

    UMLScenePolygon* m_pClearPoly;  ///< A polygon object to blank out any lines we don't want to see.
    Circle*       m_pCircle;        ///< The transparent circle required by containment associations.
    SubsetSymbol* m_pSubsetSymbol;  ///< The subset notation required by Child to Category associations.
    QPolygon      m_ParallelLines;  ///< Contains the calculated points for the parallel line on a collaboration message to use.
    Region        m_DockRegion;     ///< Region where the line docks.
    bool m_bHeadCreated;
    bool m_bSubsetSymbolCreated;
    bool m_bParallelLineCreated;

public slots:
    void slotLineColorChanged( Uml::IDType viewID );
    void slotLineWidthChanged( Uml::IDType viewID );
};

#endif

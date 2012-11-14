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

#include <QList>
#include <QObject>
#include <QPoint>

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
    typedef QList<QGraphicsLineItem*> LineList;
    typedef QList<QGraphicsRectItem*> RectList;

    AssociationLine();
    ~AssociationLine();

    UMLScenePoint point(int pointIndex) const;
    bool setPoint(int pointIndex, const UMLScenePoint &point);
    UMLScenePoint startPoint() const;
    UMLScenePoint endPoint() const;

    bool insertPoint(int pointIndex, const UMLScenePoint &point);
    bool removePoint(int pointIndex, const UMLScenePoint &point = UMLScenePoint(), unsigned short delta = 0);

    int count() const;
    void cleanup();

    int closestPointIndex(const UMLScenePoint &position);
    bool isPoint(int pointIndex, const UMLScenePoint &point, unsigned short delta = 0);

    bool setEndPoints(const UMLScenePoint &start, const UMLScenePoint &end);

    bool hasPoints() const;
    void dumpPoints();

    bool loadFromXMI(QDomElement & qElement);
    void saveToXMI(QDomDocument & qDoc, QDomElement & qElement);

    QPen pen();

    QColor lineColor();
    void setLineColor(const QColor &color);

    uint lineWidth();
    void setLineWidth(uint width);

    /**
     * Returns the Association this class is linked to.
     */
    AssociationWidget * getAssociation() {
        return m_associationWidget;
    }
    void setAssociation(AssociationWidget * association);

    Uml::AssociationType getAssocType() const;
    void setAssocType(Uml::AssociationType type);

    bool operator==(const AssociationLine & rhs);

    AssociationLine & operator=(const AssociationLine & rhs);

    enum Region {  ///< Enum to tell whether the line docks top/bottom or left/right.
        TopBottom, LeftRight
    };

    void setDockRegion(Region region);

    void setSelected(bool select);

    void activate();

    void update();

public slots:
    void slotLineColorChanged(Uml::IDType viewID);
    void slotLineWidthChanged(Uml::IDType viewID);

protected:

    /**
     * Draw a (hollow) circle.
     * We can't use QCanvasEllipse directly for this because it doesn't
     * use the pen, i.e. QCanvasEllipse only draws filled ellipses.
     */
    class Circle : public QGraphicsEllipseItem
    {
    public:
        explicit Circle(int radius = 0);
        void setRadius(int radius);
        int getRadius() const;

        void drawShape(QPainter& p);
    };

    /**
     * Draw the subset Symbol
     */
    class SubsetSymbol : public QGraphicsEllipseItem
    {
    public:
        explicit SubsetSymbol();

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

    UMLScene *canvas();

    void moveSelected(int pointIndex);

    void setupSelected();

    void calculateHead();

    void createHeadLines();
    void createSubsetSymbol();

    void updateHead();
    void updateSubsetSymbol();

    void calculateParallelLine();
    void setupParallelLine();
    void updateParallelLine();

    void growList(LineList &list, int by);

    /********Attributes*************/

    AssociationWidget* m_associationWidget;  ///< The association we are representing.
    LineList           m_LineList;      ///< Contains all the lines of the association.
    RectList           m_RectList;      ///< Selected boxes list.
    LineList           m_HeadList;      ///< Head lines.
    LineList           m_ParallelList;  ///< The parallel line.
    bool               m_bSelected;     ///< Selected status.
    QPolygonF          m_PointArray;    ///< Contains calculated points used to draw the line head.

    /**
     *   Contains calculated points used to draw the line head.
     */
    UMLScenePoint m_ArrowPointA, m_ArrowPointB, m_MidPoint, m_EgdePoint;

    QGraphicsPolygonItem* m_pClearPoly;  ///< A polygon object to blank out any lines we don't want to see.
    Circle*       m_pCircle;        ///< The transparent circle required by containment associations.
    SubsetSymbol* m_pSubsetSymbol;  ///< The subset notation required by Child to Category associations.
    QPolygonF     m_ParallelLines;  ///< Contains the calculated points for the parallel line on a collaboration message to use.
    Region        m_DockRegion;     ///< Region where the line docks.
    bool m_bHeadCreated;
    bool m_bSubsetSymbolCreated;
    bool m_bParallelLineCreated;
};

#endif

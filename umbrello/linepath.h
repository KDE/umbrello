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
#include <qobject.h>
#include <qptrlist.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qcanvas.h>
#include <qpainter.h>
#include "umlnamespace.h"

/* how many pixels a user could click around a point */
#define POINT_DELTA 5

class AssociationWidget;
class UMLView;

// Qt forward declarations
class QDataStream;
class QDomDocument;
class QDomElement;

// typedefs
typedef QPtrList<QCanvasLine> LineList;
typedef QPtrListIterator<QCanvasLine> LineListIt;

typedef QPtrList<QCanvasRectangle> RectList;
typedef QPtrListIterator<QCanvasRectangle> RectListIt;


/**
 *@author Paul Hensgen
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class LinePath : public QObject {
    Q_OBJECT
public:
    /**
    *   Constructor
    */
    LinePath();

    /**
    *   Deconstructor
    */
    ~LinePath();

    /**
    *   equal to (==) operator
    */
    bool operator==( LinePath & rhs );

    /**
    *   copy ( = ) operator
    */
    LinePath & operator=( LinePath & rhs );

    /**
    *   Enum to tell whether the line docks top/bottom or left/right
    */
    enum Region {
        TopBottom, LeftRight
    };

    /**
    *   Tell the line where the line docks
    */
    void setDockRegion( Region region );

    bool hasPoints ();
    void dumpPoints ();

    /**
    *   Returns the point at the point index.
    */
    QPoint getPoint( int pointIndex );

    /**
    *   Sets the position of an already set point.
    */
    bool setPoint( int pointIndex, const QPoint &point );

    /**
    *   Checks, if we are at an end of the segment or somewhere in the middle.
    *   We use the delta, because with the mouse it is hard to find the
    *   exactly point.
    */
    bool isPoint( int pointIndex, const QPoint &point, unsigned short delta = 0 );

    /**
    *   Inserts a point at the given index.
    */
    bool insertPoint( int pointIndex, const QPoint &point );

    /**
    *   Removes the point on the line given by the index, at the coordinates
    *   given by point with a fuzzy of delta
    */
    bool removePoint( int pointIndex, const QPoint &point, unsigned short delta = 0 );

    /**
    *   Sets the start and end points.
    */
    bool setStartEndPoints( const QPoint &start, const QPoint &end );

    /**
    *   Returns the amount of POINTS on the line.
    *   Includes start and end points.
    */
    int count();

    /**
    *   Returns -1 if the given point is not on the line.
    *   else returns the line segment the point is on.
    *   Use the value to insert points at the point position.
    */
    int onLinePath( const QPoint &position );

    /**
    *   Sets the canvas to be used.
    */
    void setCanvas( QCanvas * canvas );

    /**
    *   Sets the Association type.
    */
    void setAssocType( Uml::Association_Type type );

    /**
    *   Calls a group of methods to update the line.  Used to save you calling multiple methods.
    */
    void update();

    /**
    *   This will setup the class ready to display the line correctly.
    *   This MUST be called before you can use this class.
    */
    void setAssociation( AssociationWidget * association );

    /**
    *   Returns the Association this class is linked to.
    */
    AssociationWidget * getAssociation() {
        return m_pAssociation;
    }

    /**
    *   Sets the status of whether the line is selected or not.
    */
    void setSelected( bool select );

    void saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

    bool loadFromXMI( QDomElement & qElement );

    /**
    *   Activates the line list.
    *   This is needed because the m_pAssociation does not yet
    *   exist at the time of the LinePath::loadFromXMI call.
    *   However, this means that the points in the m_LineList
    *   do not have a parent when they are loaded.
    *   They need to be reparented by calling LinePath::activate()
    *   once the m_pAssociation exists.
    */
    void activate();

    /**
    *   Removes and item created that are no longer needed.
    */
    void cleanup();

    /**
     * Returns the type of pen to use depending on the type of Association.
     */
    QPen getPen();

    /**
    *   Sets the line color used by the line.
    */
    void setLineColor( const QColor &color );
    /**
    *   Sets the line width used by the line.
    */
    void setLineWidth( uint width );

protected:

    /**
     * Draw a (hollow) circle.
     * We can't use QCanvasEllipse directly for this because it doesn't
     * use the pen, i.e. QCanvasEllipse only draws filled ellipses.
     */
    class Circle : public QCanvasEllipse {
    public:
        explicit Circle(QCanvas * canvas, int radius = 0);
        void setRadius(int radius);
        int getRadius() const;
        void setX(int x);
        void setY(int y);
        /**
         * The beef: Override method from QCanvasEllipse.
         */
        void drawShape(QPainter& p);
    };

    /**
    *   Returns the canvas being used.
    *   Will return zero if the Association hasn't been set.
    *
    *   This class doesn't hold this information but is a wrapper
    *   method to stop calls to undefined variable like m_pAssociation.
    */
    QCanvas * getCanvas();

    /**
    *   Returns the Association type.
    *   Returns Uml::at_Association if association hasn't been set.
    *
    *   This class doesn't hold this information but is a wrapper
    *   method to stop calls to undefined variable like m_pAssociation.
    */
    Uml::Association_Type getAssocType();

    /**
    *   Returns the Line Color to use.
    *   Returns black if association not set.
    *
    *   This class doesn't hold this information but is a wrapper
    *   method to stop calls to undefined variable like m_pAssociation.
    */
    QColor getLineColor();
    /**
    *   Returns the Line Width to use.
    *   Returns 0 if association not set.
    *
    *   This class doesn't hold this information but is a wrapper
    *   method to stop calls to undefined variable like m_pAssociation.
    */
    uint getLineWidth();

    /**
    *   Moves the selected canvas widgets.
    */
    void moveSelected( int pointIndex );

    /**
    *   Sets up the selected canvases needed.
    */
    void setupSelected();

    /**
    *   Calculates the head points.
    */
    void calculateHead();

    /**
    *   Creates the head lines to display the head.
    */
    void createHeadLines();

    /**
     * Create a number of new lines and append them to the given list.
     *
     * @param list  The list into which to append lines.
     * @param by    The number of lines to insert into the given list.
     */
    void growList(LineList &list, int by);

    /**
    *   Updates the head lines.  Call after calculating the new points.
    */
    void updateHead();

    /**
    *   Creates the line objects to display the parallel line.
    */
    void setupParallelLine();

    /**
    *   Calculates the position of the parallel line.
    */
    void calculateParallelLine();

    /**
    *   Updates the parallel line.
    *   Call after calculating the new position.
    */
    void updateParallelLine();

    /********Attributes*************/

    /**
    *   The association we are representing.
    */
    AssociationWidget * m_pAssociation;

    /**
    *   Contains all the lines of the association.
    */
    LineList m_LineList;

    /**
    *   Selected boxes list.
    */
    RectList m_RectList;

    /**
    *   Head lines.
    */
    LineList m_HeadList;

    /**
    *   The parallel line.
    */
    LineList m_ParallelList;

    /**
    *     Selected status.
    */
    bool  m_bSelected;

    /**
    *   Contains calculated points used to draw the line head.
    */
    QPointArray m_PointArray;

    /**
    *   Contains calculated points used to draw the line head.
    */
    QPoint m_ArrowPointA, m_ArrowPointB, m_MidPoint, m_EgdePoint;

    /**
    *   A polygon object to blank out any lines we don't want to see.
    */
    QCanvasPolygon * m_pClearPoly;

    /**
     *  The transparent circle required by containment associations.
     */
    Circle * m_pCircle;

    /**
    *   Contains the calculated points for the parallel line
    *   on a collaboration message to use.
    */
    QPointArray m_ParallelLines;

    /**
    *   Region where the line docks
    */
    Region m_DockRegion;

    bool m_bHeadCreated;

    bool m_bParallelLineCreated;

public slots:

    /**
    *   Sets the line color used by the line.
    *
    * @param viewID The id of the object behind the widget.
    */
    void slotLineColorChanged( Uml::IDType viewID );
    /**
    *   Sets the line width used by the line.
    *
    * @param viewID The id of the object behind the widget.
    */
    void slotLineWidthChanged( Uml::IDType viewID );
};

#endif

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LINEPATH_H
#define LINEPATH_H
#include <qobject.h>
#include <qptrlist.h>
#include <qpoint.h>
#include <qpointarray.h>
#include "umlnamespace.h"

class AssociationWidget;
class UMLView;

// Qt forward declarations
class QCanvasEllipse;
class QCanvasLine;
class QCanvasPolygon;
class QCanvasRectangle;
class QCanvas;
class QDataStream;
class QDomDocument;
class QDomElement;

// typedefs
typedef QPtrList<QCanvasLine> LineList;
typedef QPtrListIterator<QCanvasLine> LineListIt;

typedef QPtrList<QCanvasRectangle> RectList;
typedef QPtrListIterator<QCanvasRectangle> RectListIt;


using namespace Uml;

/**
 *@author Paul Hensgen
 */
class LinePath : public QObject {
	Q_OBJECT
	friend class AssociationWidget;

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
	*   Returns the point at the point index.
	*/
	QPoint getPoint( int pointIndex );

	/**
	*   Sets the position of an already set point.
	*/
	bool setPoint( int pointIndex, QPoint point );

	/**
	*   Checks, if we are at an end of the segment or somewhere in the middle.
	*   We use the delta, because with the mouse it is hard to find the
	*   exactly point.
	*/
	bool isPoint( int pointIndex, QPoint point, unsigned short delta = 0 );

	/**
	*   Inserts a point at the given index.
	*/
	bool insertPoint( int pointIndex, QPoint point );

	/**
	*   Removes the point given by the index.
	*/
	bool removePoint( int pointIndex );

	/**
	*   Sets the start and end points.
	*/
	bool setStartEndPoints( QPoint start, QPoint end );

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
	int onLinePath( QPoint position );

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

	bool saveToXMI( QDomDocument & qDoc, QDomElement & qElement );

	bool loadFromXMI( QDomElement & qElement );

	/**
	*   Removes and item created that are no longer needed.
	*/
	void cleanup();

protected:

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
	*   Moves the selected canvas widgets.
	*/
	void moveSelected( int pointIndex );

	/**
	*   Sets up the selected canvases needed.
	*/
	void setupSelected();

	/**
	*   Returns the type of pen to use depending on the type of Association.
	*/
	QPen getPen();

	/**
	*   Calculates the head points.
	*/
	void calculateHead();

	/**
	*   Creates the head lines to display the head.
	*/
	void createHeadLines();

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

	/*
	*   Contains calculated points used to draw the line head.
	*/
	QPoint m_ArrowPointA, m_ArrowPointB, m_MidPoint, m_FullPoint, m_LastPoint;

	/**
	*   A polygon object to blank out any lines we don't want to see.
	*/
	QCanvasPolygon * m_pClearPoly;

	/**
	*   The Ellipse object to display, (what else) an ellipse.
	*/
	QCanvasEllipse * m_pEllipse;

	/**
	*   Contains the calculated points for the parallel line
	*   on a collaboration message to use.
	*/
	QPointArray m_ParallelLines;

	bool m_bHeadCreated;

	bool m_bParallelLineCreated;

public slots:

	/**
	*   Sets the line color used by the line.
	*/
	void setLineColor( QColor color );
};

#endif
